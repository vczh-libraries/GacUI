#include "GuiCppGen.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace stream;
		using namespace filesystem;
		using namespace workflow;
		using namespace workflow::cppcodegen;

		Ptr<GuiResourceFolder> PrecompileAndWriteErrors(
			Ptr<GuiResource> resource,
			IGuiResourcePrecompileCallback* callback,
			collections::List<GuiResourceError>& errors,
			const filesystem::FilePath& errorPath)
		{
			auto precompiledFolder = resource->Precompile(callback, errors);
			if (errors.Count() > 0)
			{
				List<WString> output;
				GuiResourceError::SortAndLog(errors, output);
				if (!File(errorPath).WriteAllLines(output, true, BomEncoder::Utf8))
				{
					return false;
				}
			}
			return precompiledFolder;
		}

		Ptr<GuiInstanceCompiledWorkflow> WriteWorkflowScript(
			Ptr<GuiResourceFolder> precompiledFolder,
			const filesystem::FilePath& workflowPath)
		{
			if (precompiledFolder)
			{
				auto compiled = precompiledFolder->GetValueByPath(L"Workflow/InstanceClass").Cast<GuiInstanceCompiledWorkflow>();
				WString text;
				if (compiled->assembly)
				{
					auto& codes = compiled->assembly->insAfterCodegen->moduleCodes;
					FOREACH_INDEXER(WString, code, codeIndex, codes)
					{
						text += L"================================(" + itow(codeIndex + 1) + L"/" + itow(codes.Count()) + L")================================\r\n";
						text += code + L"\r\n";
					}
				}
				else
				{
					FOREACH_INDEXER(GuiInstanceCompiledWorkflow::ModuleRecord, moduleRecord, codeIndex, compiled->modules)
					{
						WString code;
						{
							MemoryStream stream;
							{
								StreamWriter writer(stream);
								WfPrint(moduleRecord.module, L"", writer);
							}
							stream.SeekFromBegin(0);
							{
								StreamReader reader(stream);
								code = reader.ReadToEnd();
							}
						}
						text += L"================================(" + itow(codeIndex + 1) + L"/" + itow(compiled->modules.Count()) + L")================================\r\n";
						text += code + L"\r\n";
					}
				}

				if (File(workflowPath).WriteAllText(text))
				{
					return compiled;
				}
			}
			return nullptr;
		}

		Ptr<workflow::cppcodegen::WfCppOutput> WriteCppCodesToFile(
			Ptr<GuiInstanceCompiledWorkflow> compiled,
			Ptr<workflow::cppcodegen::WfCppInput> cppInput,
			const filesystem::FilePath& cppFolder)
		{
			auto output = GenerateCppFiles(cppInput, compiled->metadata.Obj());
			FOREACH_INDEXER(WString, fileName, index, output->cppFiles.Keys())
			{
				WString code = output->cppFiles.Values()[index];
				File file(cppFolder / fileName);

				if (file.Exists())
				{
					WString inputText;
					BomEncoder::Encoding inputEncoding;
					bool inputBom;
					file.ReadAllTextWithEncodingTesting(inputText, inputEncoding, inputBom);
					code = MergeCppFileContent(inputText, code);
				}

				if (file.Exists())
				{
					WString inputText;
					BomEncoder::Encoding inputEncoding;
					bool inputBom;
					file.ReadAllTextWithEncodingTesting(inputText, inputEncoding, inputBom);
					if (inputText == code)
					{
						continue;
					}
				}
				file.WriteAllText(code, true, BomEncoder::Utf8);
			}
			return output;
		}

		bool WriteBinaryResource(
			Ptr<GuiResource> resource,
			bool compress,
			bool workflow,
			const filesystem::FilePath& filePath)
		{
			auto folder = resource->GetFolder(L"Precompiled")->GetFolder(L"Workflow");
			if (!workflow)
			{
				resource->GetFolder(L"Precompiled")->RemoveFolder(L"Workflow");
			}

			FileStream fileStream(filePath.GetFullPath(), FileStream::WriteOnly);

			if (fileStream.IsAvailable())
			{
				if (compress)
				{
					LzwEncoder encoder;
					EncoderStream encoderStream(fileStream, encoder);
					resource->SavePrecompiledBinary(encoderStream);
				}
				else
				{
					resource->SavePrecompiledBinary(fileStream);
				}
			}

			if (folder && !workflow)
			{
				resource->GetFolder(L"Precompiled")->AddFolder(L"Workflow", folder);
			}

			return fileStream.IsAvailable();
		}

		void WriteEmbeddedBinaryClass(MemoryStream& binaryStream, const WString& className, const WString& prefix, StreamWriter& writer)
		{
			binaryStream.SeekFromBegin(0);
			vint lengthBeforeCompressing = (vint)binaryStream.Size();

			MemoryStream compressedStream;
			{
				LzwEncoder encoder;
				EncoderStream encoderStream(compressedStream, encoder);
				while (true)
				{
					char buffer[1024];
					vint copied = binaryStream.Read(buffer, (vint)sizeof(buffer));
					if (copied == 0)
					{
						break;
					}
					encoderStream.Write(buffer, copied);
				}
			}

			compressedStream.SeekFromBegin(0);
			vint length = (vint)compressedStream.Size();
			const vint block = 1024;
			vint remain = length % block;
			vint rows = length / block + (remain ? 1 : 0);

#define PREFIX writer.WriteString(prefix);

			PREFIX writer.WriteLine(L"class " + className);
			PREFIX writer.WriteLine(L"{");
			PREFIX writer.WriteLine(L"public:");

			PREFIX writer.WriteLine(L"\tstatic const vint parserBufferLength = " + itow(length) + L"; // " + itow(lengthBeforeCompressing) + L" bytes before compressing");
			PREFIX writer.WriteLine(L"\tstatic const vint parserBufferBlock = " + itow(block) + L";");
			PREFIX writer.WriteLine(L"\tstatic const vint parserBufferRemain = " + itow(remain) + L";");
			PREFIX writer.WriteLine(L"\tstatic const vint parserBufferRows = " + itow(rows) + L";");
			PREFIX writer.WriteLine(L"\tstatic const char* parserBuffer[" + itow(rows) + L"];");
			writer.WriteLine(L"");

			PREFIX writer.WriteLine(L"\tstatic void ReadToStream(vl::stream::MemoryStream& stream)");
			PREFIX writer.WriteLine(L"\t{");
			PREFIX writer.WriteLine(L"\t\tvl::stream::MemoryStream compressedStream;");
			PREFIX writer.WriteLine(L"\t\tfor (vint i = 0; i < parserBufferRows; i++)");
			PREFIX writer.WriteLine(L"\t\t{");
			PREFIX writer.WriteLine(L"\t\t\tvint size = i == parserBufferRows - 1 ? parserBufferRemain : parserBufferBlock;");
			PREFIX writer.WriteLine(L"\t\t\tcompressedStream.Write((void*)parserBuffer[i], size);");
			PREFIX writer.WriteLine(L"\t\t}");
			PREFIX writer.WriteLine(L"\t\tcompressedStream.SeekFromBegin(0);");
			PREFIX writer.WriteLine(L"\t\tvl::stream::LzwDecoder decoder;");
			PREFIX writer.WriteLine(L"\t\tvl::stream::DecoderStream decoderStream(compressedStream, decoder);");
			PREFIX writer.WriteLine(L"\t\tvl::collections::Array<vl::vuint8_t> buffer(65536);");
			PREFIX writer.WriteLine(L"\t\twhile (true)");
			PREFIX writer.WriteLine(L"\t\t{");
			PREFIX writer.WriteLine(L"\t\t\tvl::vint size = decoderStream.Read(&buffer[0], 65536);");
			PREFIX writer.WriteLine(L"\t\t\tif (size == 0) break;");
			PREFIX writer.WriteLine(L"\t\t\tstream.Write(&buffer[0], size);");
			PREFIX writer.WriteLine(L"\t\t}");
			PREFIX writer.WriteLine(L"\t\tstream.SeekFromBegin(0);");
			PREFIX writer.WriteLine(L"\t}");

			PREFIX writer.WriteLine(L"};");
			writer.WriteLine(L"");
			PREFIX writer.WriteLine(L"const char* " + className + L"::parserBuffer[] = {");

			char buffer[block];
			const wchar_t* hex = L"0123456789ABCDEF";
			for (vint i = 0; i < rows; i++)
			{
				vint size = i == rows - 1 ? remain : block;
				compressedStream.Read(buffer, size);
				PREFIX writer.WriteString(L"\t\"");
				for (vint j = 0; j < size; j++)
				{
					vuint8_t byte = buffer[j];
					writer.WriteString(L"\\x");
					writer.WriteChar(hex[byte / 16]);
					writer.WriteChar(hex[byte % 16]);
				}
				writer.WriteLine(L"\",");
			}
			PREFIX writer.WriteLine(L"\t};");

#undef PREFIX
		}

		bool WriteEmbeddedResource(Ptr<GuiResource> resource,
			Ptr<workflow::cppcodegen::WfCppInput> cppInput,
			Ptr<workflow::cppcodegen::WfCppOutput> cppOutput,
			const filesystem::FilePath& filePath)
		{
			WString code;
			MemoryStream stream;
			{
				StreamWriter writer(stream);

				writer.WriteLine(L"#include \"" + cppOutput->entryFileName + L".h\"");
				writer.WriteLine(L"");
				writer.WriteLine(L"namespace vl");
				writer.WriteLine(L"{");
				writer.WriteLine(L"\tnamespace presentation");
				writer.WriteLine(L"\t{");
				writer.WriteLine(L"\t\tnamespace user_resource");
				writer.WriteLine(L"\t\t{");
				writer.WriteLine(L"\t\t\tusing namespace collections;");
				writer.WriteLine(L"\t\t\tusing namespace stream;");
				writer.WriteLine(L"\t\t\tusing namespace controls;");
				writer.WriteLine(L"");

				{
					MemoryStream resourceStream;
					{
						auto folder = resource->GetFolder(L"Precompiled")->GetFolder(L"Workflow");
						resource->SavePrecompiledBinary(resourceStream);
						resource->GetFolder(L"Precompiled")->AddFolder(L"Workflow", folder);
					}
					WriteEmbeddedBinaryClass(resourceStream, cppInput->assemblyName + L"ResourceReader", L"\t\t\t", writer);
					writer.WriteLine(L"");
				}
				{
					writer.WriteLine(L"\t\t\tclass " + cppInput->assemblyName + L"ResourceLoaderPlugin : public Object, public IGuiPlugin");
					writer.WriteLine(L"\t\t\t{");
					writer.WriteLine(L"\t\t\tpublic:");
					writer.WriteLine(L"");
					writer.WriteLine(L"\t\t\t\tGUI_PLUGIN_NAME(GacGen_" + cppInput->assemblyName + L"ResourceLoader)");
					writer.WriteLine(L"\t\t\t\t{");
					writer.WriteLine(L"\t\t\t\t\tGUI_PLUGIN_DEPEND(GacUI_Resource);");
					writer.WriteLine(L"#ifdef VCZH_DEBUG_NO_REFLECTION");
					writer.WriteLine(L"\t\t\t\t\tGUI_PLUGIN_DEPEND(GacUI_Reflection);");
					writer.WriteLine(L"\t\t\t\t\tGUI_PLUGIN_DEPEND(GacUI_Compiler_RuntimeTypeResolvers);");
					writer.WriteLine(L"#endif");
					writer.WriteLine(L"\t\t\t\t}");
					writer.WriteLine(L"");
					writer.WriteLine(L"\t\t\t\tvoid Load()override");
					writer.WriteLine(L"\t\t\t\t{");
					writer.WriteLine(L"\t\t\t\t\tList<GuiResourceError> errors;");
					writer.WriteLine(L"\t\t\t\t\tMemoryStream resourceStream;");
					writer.WriteLine(L"\t\t\t\t\t" + cppInput->assemblyName + L"ResourceReader::ReadToStream(resourceStream);");
					writer.WriteLine(L"\t\t\t\t\tauto resource = GuiResource::LoadPrecompiledBinary(resourceStream, errors);");
					writer.WriteLine(L"\t\t\t\t\tGetResourceManager()->SetResource(L\"" + cppInput->assemblyName + L"\", resource, GuiResourceUsage::InstanceClass);");
					writer.WriteLine(L"\t\t\t\t}");
					writer.WriteLine(L"");
					writer.WriteLine(L"\t\t\t\tvoid Unload()override");
					writer.WriteLine(L"\t\t\t\t{");
					writer.WriteLine(L"\t\t\t\t}");
					writer.WriteLine(L"\t\t\t};");
					writer.WriteLine(L"\t\t\tGUI_REGISTER_PLUGIN(" + cppInput->assemblyName + L"ResourceLoaderPlugin)");
				}

				writer.WriteLine(L"\t\t}");
				writer.WriteLine(L"\t}");
				writer.WriteLine(L"}");
			}
			stream.SeekFromBegin(0);
			{
				StreamReader reader(stream);
				code = reader.ReadToEnd();
			}

			File file(filePath);
			if (file.Exists())
			{
				WString inputText;
				BomEncoder::Encoding inputEncoding;
				bool inputBom;
				file.ReadAllTextWithEncodingTesting(inputText, inputEncoding, inputBom);
				if (inputText == code)
				{
					return true;
				}
			}
			return file.WriteAllText(code, true, BomEncoder::Utf8);
		}
	}
}