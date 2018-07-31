#include "GuiCppGen.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace stream;
		using namespace filesystem;
		using namespace parsing;
		using namespace workflow;
		using namespace workflow::cppcodegen;

		bool WriteErrors(
			collections::List<GuiResourceError>& errors,
			const filesystem::FilePath& errorPath)
		{
			List<WString> output;
			GuiResourceError::SortAndLog(errors, output);
			return File(errorPath).WriteAllLines(output, true, BomEncoder::Utf8);
		}

		Ptr<GuiResourceFolder> PrecompileResource(
			Ptr<GuiResource> resource,
			IGuiResourcePrecompileCallback* callback,
			collections::List<GuiResourceError>& errors)
		{
			auto precompiledFolder = resource->Precompile(callback, errors);
			return precompiledFolder;
		}

		Ptr<GuiInstanceCompiledWorkflow> WriteWorkflowScript(
			Ptr<GuiResourceFolder> precompiledFolder,
			const WString& assemblyResourcePath,
			const filesystem::FilePath& workflowPath)
		{
			if (precompiledFolder)
			{
				if (auto compiled = precompiledFolder->GetValueByPath(assemblyResourcePath).Cast<GuiInstanceCompiledWorkflow>())
				{
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
							WString code = GenerateToStream([&](StreamWriter& writer)
							{
								WfPrint(moduleRecord.module, L"", writer);
							});
							text += L"================================(" + itow(codeIndex + 1) + L"/" + itow(compiled->modules.Count()) + L")================================\r\n";
							text += code + L"\r\n";
						}
					}

					if (File(workflowPath).WriteAllText(text))
					{
						return compiled;
					}
				}
			}
			return nullptr;
		}

		Ptr<workflow::cppcodegen::WfCppOutput> WriteCppCodesToFile(
			Ptr<GuiResource> resource,
			Ptr<GuiInstanceCompiledWorkflow> compiled,
			Ptr<workflow::cppcodegen::WfCppInput> cppInput,
			const filesystem::FilePath& cppFolder,
			collections::List<GuiResourceError>& errors)
		{
			auto output = GenerateCppFiles(cppInput, compiled->metadata.Obj());

			if (compiled->metadata->errors.Count() > 0)
			{
				FOREACH(Ptr<ParsingError>, error, compiled->metadata->errors)
				{
					errors.Add(GuiResourceError({ {resource} }, error->errorMessage));
				}
				return nullptr;
			}

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
			bool includeAssemblyInResource,
			Nullable<filesystem::FilePath> resourceOutput,
			Nullable<filesystem::FilePath> assemblyOutput)
		{
			auto precompiled = resource->GetFolder(L"Precompiled");
			auto folder = precompiled->GetFolder(L"Workflow");

			if (resourceOutput)
			{
				FileStream fileStream(resourceOutput.Value().GetFullPath(), FileStream::WriteOnly);
				if (!fileStream.IsAvailable()) return false;

				if (!includeAssemblyInResource) precompiled->RemoveFolder(L"Workflow");
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
				if (!includeAssemblyInResource) precompiled->AddFolder(L"Workflow", folder);
			}

			if (assemblyOutput)
			{
				if (auto item = folder->GetItem(L"InstanceClass"))
				{
					if (auto compiled = item->GetContent().Cast<GuiInstanceCompiledWorkflow>())
					{
						if (compiled->assembly)
						{
							FileStream fileStream(assemblyOutput.Value().GetFullPath(), FileStream::WriteOnly);
							if (!fileStream.IsAvailable()) return false;
							compiled->assembly->Serialize(fileStream);
						}
					}
				}
			}

			return true;
		}

		void WriteEmbeddedBinaryClass(MemoryStream& binaryStream, bool compress, const WString& className, const WString& prefix, StreamWriter& writer)
		{
			MemoryStream compressedStream;
			binaryStream.SeekFromBegin(0);
			if (compress)
			{
				CompressStream(binaryStream, compressedStream);
			}
			else
			{
				CopyStream(binaryStream, compressedStream);
			}
			compressedStream.SeekFromBegin(0);

			vint lengthBeforeCompressing = (vint)binaryStream.Size();
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
			PREFIX writer.WriteLine(L"\t\tDecompressStream(parserBuffer, " + WString(compress ? L"true" : L"false") + L", parserBufferRows, parserBufferBlock, parserBufferRemain, stream);");
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
			bool compress,
			const filesystem::FilePath& filePath)
		{
			WString code = GenerateToStream([&](StreamWriter& writer)
			{
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
						auto precompiled = resource->GetFolder(L"Precompiled");
						auto folder = precompiled->GetFolder(L"Workflow");
						precompiled->RemoveFolder(L"Workflow");
						resource->SavePrecompiledBinary(resourceStream);
						precompiled->AddFolder(L"Workflow", folder);
					}
					WriteEmbeddedBinaryClass(resourceStream, compress, cppInput->assemblyName + L"ResourceReader", L"\t\t\t", writer);
					writer.WriteLine(L"");
				}
				{
					writer.WriteLine(L"\t\t\tclass " + cppInput->assemblyName + L"ResourceLoaderPlugin : public Object, public IGuiPlugin");
					writer.WriteLine(L"\t\t\t{");
					writer.WriteLine(L"\t\t\tpublic:");
					writer.WriteLine(L"");
					writer.WriteLine(L"\t\t\t\tGUI_PLUGIN_NAME(GacGen_" + cppInput->assemblyName + L"ResourceLoader)");
					writer.WriteLine(L"\t\t\t\t{");
					writer.WriteLine(L"\t\t\t\t\tGUI_PLUGIN_DEPEND(GacUI_Res_Resource);");
					writer.WriteLine(L"\t\t\t\t\tGUI_PLUGIN_DEPEND(GacUI_Res_TypeResolvers);");
					writer.WriteLine(L"#ifndef VCZH_DEBUG_NO_REFLECTION");
					writer.WriteLine(L"\t\t\t\t\tGUI_PLUGIN_DEPEND(GacUI_Instance_Reflection);");
					writer.WriteLine(L"\t\t\t\t\tGUI_PLUGIN_DEPEND(GacUI_Compiler_WorkflowTypeResolvers);");
					writer.WriteLine(L"#endif");
					writer.WriteLine(L"\t\t\t\t}");
					writer.WriteLine(L"");
					writer.WriteLine(L"\t\t\t\tvoid Load()override");
					writer.WriteLine(L"\t\t\t\t{");
					writer.WriteLine(L"\t\t\t\t\tList<GuiResourceError> errors;");
					writer.WriteLine(L"\t\t\t\t\tMemoryStream resourceStream;");
					writer.WriteLine(L"\t\t\t\t\t" + cppInput->assemblyName + L"ResourceReader::ReadToStream(resourceStream);");
					writer.WriteLine(L"\t\t\t\t\tresourceStream.SeekFromBegin(0);");
					writer.WriteLine(L"\t\t\t\t\tGetResourceManager()->LoadResourceOrPending(resourceStream, GuiResourceUsage::InstanceClass);");
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
			});

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
