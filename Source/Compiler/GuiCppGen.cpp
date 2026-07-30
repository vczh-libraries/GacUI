#include "GuiCppGen.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace stream;
		using namespace filesystem;
		using namespace workflow;
		using namespace workflow::analyzer;
		using namespace workflow::cppcodegen;

		void AddCppGenerationError(Ptr<GuiResource> resource, collections::List<GuiResourceError>& errors, const WString& message)
		{
			errors.Add(GuiResourceError({ resource }, message));
		}

		void AddWorkflowErrors(Ptr<GuiResource> resource, WfLexicalScopeManager* manager, collections::List<GuiResourceError>& errors)
		{
			for (auto error : manager->errors)
			{
				AddCppGenerationError(resource, errors, error.message);
			}
		}

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
			GuiResourceCpuArchitecture targetCpuArchitecture,
			IGuiResourcePrecompileCallback* callback,
			collections::List<GuiResourceError>& errors)
		{
			auto precompiledFolder = resource->Precompile(targetCpuArchitecture, callback, errors);
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
						for (auto [code, codeIndex] : indexed(codes))
						{
							text += L"================================(" + itow(codeIndex + 1) + L"/" + itow(codes.Count()) + L")================================\r\n";
							text += code + L"\r\n";
						}
					}
					else
					{
						for (auto [moduleRecord, codeIndex] : indexed(compiled->modules))
						{
							WString code = GenerateToStream([&, module = moduleRecord.module](StreamWriter& writer)
							{
								WfPrint(module, L"", writer);
							});
							text += L"================================(" + itow(codeIndex + 1) + L"/" + itow(compiled->modules.Count()) + L")================================\r\n";
							text += code + L"\r\n";
						}
					}

					if (File(workflowPath).WriteAllText(text, true, BomEncoder::Utf8))
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
			if (!compiled || !compiled->metadata)
			{
				AddCppGenerationError(resource, errors, L"Workflow C++ generation requires a compiled InstanceClass manager.");
				return nullptr;
			}

			auto output = GenerateCppFiles(cppInput, compiled->metadata.Obj());

			if (compiled->metadata->errors.Count() > 0)
			{
				AddWorkflowErrors(resource, compiled->metadata.Obj(), errors);
				return nullptr;
			}
			if (!output)
			{
				AddCppGenerationError(resource, errors, L"Workflow C++ generation returned no output.");
				return nullptr;
			}

			// TODO: (enumerable) foreach on dictionary
			for (auto [fileName, index] : indexed(output->cppFiles.Keys()))
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

		bool HasRpcMetadata(Ptr<GuiInstanceCompiledWorkflow> compiled)
		{
			return compiled
				&& compiled->metadata
				&& compiled->metadata->rpcMetadata
				&& compiled->metadata->rpcMetadata->metadataModule;
		}

		bool ValidateRpcCppGenerationConfiguration(
			Ptr<GuiResource> resource,
			Ptr<GuiInstanceCompiledWorkflow> compiled,
			bool hasCppConfiguration,
			collections::List<GuiResourceError>& errors)
		{
			if (HasRpcMetadata(compiled) && !hasCppConfiguration)
			{
				AddCppGenerationError(resource, errors, L"Validated RPC metadata requires GacGenConfig/Cpp to define the generated C++ name and source folder.");
				return false;
			}
			return true;
		}

		WString PrintRpcModules(Ptr<WfModule> wrapperModule, Ptr<WfModule> wrapperJsonModule)
		{
			return GenerateToStream([&](StreamWriter& writer)
			{
				writer.WriteLine(L"================================(1/2: RPC Wrapper)================================");
				WfPrint(wrapperModule, L"", writer);
				writer.WriteLine(L"");
				writer.WriteLine(L"================================(2/2: JSON RPC Wrapper)===========================");
				WfPrint(wrapperJsonModule, L"", writer);
				writer.WriteLine(L"");
			});
		}

		Ptr<GuiResourceRpcCppOutput> GenerateRpcCppOutput(
			Ptr<GuiResource> resource,
			Ptr<GuiInstanceCompiledWorkflow> compiled,
			Ptr<workflow::cppcodegen::WfCppOutput> normalOutput,
			const WString& assemblyName,
			const WString& cppComment,
			workflow::IWfCompilerCallback* compilerCallback,
			collections::List<GuiResourceError>& errors)
		{
			if (!HasRpcMetadata(compiled))
			{
				AddCppGenerationError(resource, errors, L"RPC C++ generation requires validated RPC metadata.");
				return nullptr;
			}
			if (!normalOutput)
			{
				AddCppGenerationError(resource, errors, L"RPC C++ generation requires the ordinary C++ output.");
				return nullptr;
			}

			auto manager = compiled->metadata;
			auto result = Ptr(new GuiResourceRpcCppOutput);
			result->wrapperModule = GenerateModuleRpc(manager.Obj(), assemblyName);
			if (manager->errors.Count() > 0)
			{
				AddWorkflowErrors(resource, manager.Obj(), errors);
				return nullptr;
			}
			if (!result->wrapperModule)
			{
				AddCppGenerationError(resource, errors, L"RPC wrapper generation returned no module.");
				return nullptr;
			}

			result->wrapperJsonModule = GenerateModuleRpcJson(manager.Obj(), assemblyName);
			if (manager->errors.Count() > 0)
			{
				AddWorkflowErrors(resource, manager.Obj(), errors);
				return nullptr;
			}
			if (!result->wrapperJsonModule)
			{
				AddCppGenerationError(resource, errors, L"JSON RPC wrapper generation returned no module.");
				return nullptr;
			}
			result->workflowCode = PrintRpcModules(result->wrapperModule, result->wrapperJsonModule);

			auto metadataModule = CopyAndClearRpcMetadata(manager->rpcMetadata->metadataModule);
			if (!metadataModule)
			{
				AddCppGenerationError(resource, errors, L"RPC metadata copying returned no module.");
				return nullptr;
			}

			auto rpcManager = Ptr(new WfLexicalScopeManager(manager->workflowParser, manager->cpuArchitecture));
			rpcManager->AddModule(metadataModule);
			rpcManager->AddModule(result->wrapperModule);
			rpcManager->AddModule(result->wrapperJsonModule);
			rpcManager->Rebuild(true, compilerCallback, false);
			if (rpcManager->errors.Count() > 0)
			{
				AddWorkflowErrors(resource, rpcManager.Obj(), errors);
				return nullptr;
			}

			auto input = Ptr(new WfCppInput(assemblyName));
			input->multiFile = WfCppFileSwitch::Disabled;
			input->reflection = WfCppFileSwitch::Disabled;
			input->defaultFileName = assemblyName;
			input->comment = cppComment;
			input->normalIncludes.Add(normalOutput->entryFileName + L".h");
			result->cppOutput = GenerateCppFiles(input, rpcManager.Obj());
			if (rpcManager->errors.Count() > 0)
			{
				AddWorkflowErrors(resource, rpcManager.Obj(), errors);
				return nullptr;
			}
			if (!result->cppOutput)
			{
				AddCppGenerationError(resource, errors, L"RPC C++ generation returned no output.");
				return nullptr;
			}

			auto expectedHeader = assemblyName + L".h";
			auto expectedCpp = assemblyName + L".cpp";
			if (result->cppOutput->entryFileName != assemblyName
				|| result->cppOutput->multiFile
				|| result->cppOutput->reflection
				|| result->cppOutput->cppFiles.Count() != 2
				|| !result->cppOutput->cppFiles.Keys().Contains(expectedHeader)
				|| !result->cppOutput->cppFiles.Keys().Contains(expectedCpp))
			{
				AddCppGenerationError(resource, errors, L"RPC C++ generation must produce exactly \"" + expectedHeader + L"\" and \"" + expectedCpp + L"\" without reflection or includes files.");
				return nullptr;
			}

			auto cppCode = result->cppOutput->cppFiles[expectedCpp];
			if (INVLOC.FindFirst(cppCode, L"namespace vl_workflow_global", Locale::None).key == -1
				|| INVLOC.FindFirst(cppCode, assemblyName + L"& " + assemblyName + L"::Instance()", Locale::None).key == -1)
			{
				AddCppGenerationError(resource, errors, L"RPC C++ generation did not produce the required global entry \"vl_workflow_global::" + assemblyName + L"::Instance()\".");
				return nullptr;
			}
			return result;
		}

		bool WriteRpcWorkflowScript(
			Ptr<GuiResource> resource,
			Ptr<GuiResourceRpcCppOutput> rpcOutput,
			const filesystem::FilePath& workflowPath,
			collections::List<GuiResourceError>& errors)
		{
			if (!rpcOutput || !File(workflowPath).WriteAllText(rpcOutput->workflowCode, true, BomEncoder::Utf8))
			{
				AddCppGenerationError(resource, errors, L"Unable to write RPC Workflow output: " + workflowPath.GetFullPath());
				return false;
			}
			return true;
		}

		bool DeleteFileIfExists(const FilePath& filePath)
		{
			File file(filePath);
			return !file.Exists() || file.Delete();
		}

		bool DeleteRpcCppFilesByPostfix(
			const filesystem::FilePath& cppFolder,
			const WString& assemblyName,
			const wchar_t* const* postfixes,
			vint count)
		{
			bool succeeded = true;
			for (vint i = 0; i < count; i++)
			{
				if (!DeleteFileIfExists(cppFolder / (assemblyName + postfixes[i])))
				{
					succeeded = false;
				}
			}
			return succeeded;
		}

		bool CleanLegacyRpcCppFiles(const filesystem::FilePath& cppFolder, const WString& assemblyName)
		{
			const wchar_t* postfixes[] =
			{
				L"Reflection.h",
				L"Reflection.cpp",
				L"Includes.h",
			};
			return DeleteRpcCppFilesByPostfix(cppFolder, assemblyName, postfixes, sizeof(postfixes) / sizeof(*postfixes));
		}

		bool CleanRpcCppTempFiles(const filesystem::FilePath& cppFolder, const WString& assemblyName)
		{
			const wchar_t* postfixes[] =
			{
				L".h.gacui.tmp",
				L".cpp.gacui.tmp",
			};
			return DeleteRpcCppFilesByPostfix(cppFolder, assemblyName, postfixes, sizeof(postfixes) / sizeof(*postfixes));
		}

		bool CleanRpcCppTransactionFiles(const filesystem::FilePath& cppFolder, const WString& assemblyName)
		{
			const wchar_t* postfixes[] =
			{
				L".h.gacui.tmp",
				L".cpp.gacui.tmp",
				L".h.gacui.backup",
				L".cpp.gacui.backup",
			};
			return DeleteRpcCppFilesByPostfix(cppFolder, assemblyName, postfixes, sizeof(postfixes) / sizeof(*postfixes));
		}

		bool CleanRpcCppAuxiliaryFiles(const filesystem::FilePath& cppFolder, const WString& assemblyName)
		{
			bool legacySucceeded = CleanLegacyRpcCppFiles(cppFolder, assemblyName);
			bool transactionSucceeded = CleanRpcCppTransactionFiles(cppFolder, assemblyName);
			return legacySucceeded && transactionSucceeded;
		}

		bool CleanRpcCppFiles(const filesystem::FilePath& cppFolder, const WString& assemblyName)
		{
			const wchar_t* postfixes[] =
			{
				L".h",
				L".cpp",
			};
			bool pairSucceeded = DeleteRpcCppFilesByPostfix(cppFolder, assemblyName, postfixes, sizeof(postfixes) / sizeof(*postfixes));
			bool auxiliarySucceeded = CleanRpcCppAuxiliaryFiles(cppFolder, assemblyName);
			return pairSucceeded && auxiliarySucceeded;
		}

		bool WriteRpcCppCodePairToFile(
			Ptr<GuiResource> resource,
			const WString& assemblyName,
			const filesystem::FilePath& cppFolder,
			const WString& generatedHeaderCode,
			const WString& generatedCppCode,
			collections::List<GuiResourceError>& errors)
		{
			auto headerName = assemblyName + L".h";
			auto cppName = assemblyName + L".cpp";
			auto prepareCode = [&](const WString& fileName, const WString& generatedCode)
			{
				auto code = generatedCode;
				File file(cppFolder / fileName);
				if (file.Exists())
				{
					code = MergeCppFileContent(file.ReadAllTextByBom(), code);
				}
				return code;
			};

			auto headerCode = prepareCode(headerName, generatedHeaderCode);
			auto cppCode = prepareCode(cppName, generatedCppCode);
			FilePath headerPath = cppFolder / headerName;
			FilePath cppPath = cppFolder / cppName;
			File headerFile(headerPath);
			File cppFile(cppPath);
			if (headerFile.Exists() && cppFile.Exists()
				&& headerFile.ReadAllTextByBom() == headerCode
				&& cppFile.ReadAllTextByBom() == cppCode)
			{
				if (!CleanRpcCppAuxiliaryFiles(cppFolder, assemblyName))
				{
					AddCppGenerationError(resource, errors, L"Unable to clean legacy or transactional RPC C++ files in: " + cppFolder.GetFullPath());
					return false;
				}
				return true;
			}

			FilePath headerTempPath = cppFolder / (headerName + L".gacui.tmp");
			FilePath cppTempPath = cppFolder / (cppName + L".gacui.tmp");
			FilePath headerBackupPath = cppFolder / (headerName + L".gacui.backup");
			FilePath cppBackupPath = cppFolder / (cppName + L".gacui.backup");
			if (!CleanRpcCppTransactionFiles(cppFolder, assemblyName))
			{
				AddCppGenerationError(resource, errors, L"Unable to prepare transactional RPC C++ output in: " + cppFolder.GetFullPath());
				return false;
			}

			auto reportTransactionFailure = [&](const WString& message, bool rollbackSucceeded, bool cleanupSucceeded)
			{
				auto fullMessage = message;
				if (!rollbackSucceeded)
				{
					fullMessage += L" Rollback failed; preserved backup files may remain in: " + cppFolder.GetFullPath();
				}
				if (!cleanupSucceeded)
				{
					fullMessage += L" Transaction artifact cleanup failed in: " + cppFolder.GetFullPath();
				}
				AddCppGenerationError(resource, errors, fullMessage);
				return false;
			};

			File headerTempFile(headerTempPath);
			File cppTempFile(cppTempPath);
			if (!headerTempFile.WriteAllText(headerCode, true, BomEncoder::Utf8)
				|| !cppTempFile.WriteAllText(cppCode, true, BomEncoder::Utf8))
			{
				auto cleanupSucceeded = CleanRpcCppTempFiles(cppFolder, assemblyName);
				return reportTransactionFailure(
					L"Unable to stage transactional RPC C++ output in: " + cppFolder.GetFullPath(),
					true,
					cleanupSucceeded
					);
			}

			bool hadHeader = headerFile.Exists();
			bool hadCpp = cppFile.Exists();
			if (hadHeader && !headerFile.Rename(headerBackupPath.GetName()))
			{
				auto cleanupSucceeded = CleanRpcCppTempFiles(cppFolder, assemblyName);
				return reportTransactionFailure(
					L"Unable to preserve the existing RPC header: " + headerPath.GetFullPath(),
					true,
					cleanupSucceeded
					);
			}
			if (hadCpp && !cppFile.Rename(cppBackupPath.GetName()))
			{
				bool rollbackSucceeded = true;
				if (hadHeader)
				{
					rollbackSucceeded = File(headerBackupPath).Rename(headerName);
				}
				auto cleanupSucceeded = rollbackSucceeded
					? CleanRpcCppTransactionFiles(cppFolder, assemblyName)
					: CleanRpcCppTempFiles(cppFolder, assemblyName);
				return reportTransactionFailure(
					L"Unable to preserve the existing RPC source: " + cppPath.GetFullPath(),
					rollbackSucceeded,
					cleanupSucceeded
					);
			}

			auto restoreBackups = [&]()
			{
				bool succeeded = true;
				if (!DeleteFileIfExists(headerPath))
				{
					succeeded = false;
				}
				if (!DeleteFileIfExists(cppPath))
				{
					succeeded = false;
				}
				if (hadHeader)
				{
					if (!File(headerBackupPath).Rename(headerName))
					{
						succeeded = false;
					}
				}
				if (hadCpp)
				{
					if (!File(cppBackupPath).Rename(cppName))
					{
						succeeded = false;
					}
				}
				return succeeded;
			};

			if (!headerTempFile.Rename(headerName))
			{
				auto rollbackSucceeded = restoreBackups();
				auto cleanupSucceeded = rollbackSucceeded
					? CleanRpcCppTransactionFiles(cppFolder, assemblyName)
					: CleanRpcCppTempFiles(cppFolder, assemblyName);
				return reportTransactionFailure(
					L"Unable to install the generated RPC header: " + headerPath.GetFullPath(),
					rollbackSucceeded,
					cleanupSucceeded
					);
			}
			if (!cppTempFile.Rename(cppName))
			{
				auto rollbackSucceeded = restoreBackups();
				auto cleanupSucceeded = rollbackSucceeded
					? CleanRpcCppTransactionFiles(cppFolder, assemblyName)
					: CleanRpcCppTempFiles(cppFolder, assemblyName);
				return reportTransactionFailure(
					L"Unable to install the generated RPC source: " + cppPath.GetFullPath(),
					rollbackSucceeded,
					cleanupSucceeded
					);
			}

			if (!CleanLegacyRpcCppFiles(cppFolder, assemblyName))
			{
				auto rollbackSucceeded = restoreBackups();
				auto cleanupSucceeded = rollbackSucceeded
					? CleanRpcCppTransactionFiles(cppFolder, assemblyName)
					: CleanRpcCppTempFiles(cppFolder, assemblyName);
				return reportTransactionFailure(
					L"Unable to clean legacy RPC C++ files in: " + cppFolder.GetFullPath(),
					rollbackSucceeded,
					cleanupSucceeded
					);
			}
			if (!CleanRpcCppTransactionFiles(cppFolder, assemblyName))
			{
				return reportTransactionFailure(
					L"Unable to finalize transactional RPC C++ output in: " + cppFolder.GetFullPath(),
					true,
					false
					);
			}
			return true;
		}

		bool GetRpcCppCodePair(
			Ptr<GuiResource> resource,
			Ptr<GuiResourceRpcCppOutput> rpcOutput,
			const WString& assemblyName,
			WString& headerCode,
			WString& cppCode,
			collections::List<GuiResourceError>& errors)
		{
			if (!rpcOutput || !rpcOutput->cppOutput)
			{
				AddCppGenerationError(resource, errors, L"Unable to write an empty RPC C++ output.");
				return false;
			}

			auto headerName = assemblyName + L".h";
			auto cppName = assemblyName + L".cpp";
			if (rpcOutput->cppOutput->cppFiles.Count() != 2
				|| !rpcOutput->cppOutput->cppFiles.Keys().Contains(headerName)
				|| !rpcOutput->cppOutput->cppFiles.Keys().Contains(cppName))
			{
				AddCppGenerationError(resource, errors, L"RPC C++ output contains an unexpected file set.");
				return false;
			}

			headerCode = rpcOutput->cppOutput->cppFiles[headerName];
			cppCode = rpcOutput->cppOutput->cppFiles[cppName];
			return true;
		}

		bool WriteRpcCppCodesToFile(
			Ptr<GuiResource> resource,
			Ptr<GuiResourceRpcCppOutput> rpcOutput,
			const WString& assemblyName,
			const filesystem::FilePath& cppFolder,
			collections::List<GuiResourceError>& errors)
		{
			WString headerCode;
			WString cppCode;
			if (!GetRpcCppCodePair(resource, rpcOutput, assemblyName, headerCode, cppCode, errors))
			{
				return false;
			}
			return WriteRpcCppCodePairToFile(resource, assemblyName, cppFolder, headerCode, cppCode, errors);
		}

		bool WriteRpcCppCodesToFileMultiPlatform(
			Ptr<GuiResource> resource,
			Ptr<GuiResourceRpcCppOutput> rpcOutput32,
			Ptr<GuiResourceRpcCppOutput> rpcOutput64,
			const WString& assemblyName,
			const filesystem::FilePath& cppFolder,
			collections::List<GuiResourceError>& errors)
		{
			WString headerCode32;
			WString cppCode32;
			WString headerCode64;
			WString cppCode64;
			if (!GetRpcCppCodePair(resource, rpcOutput32, assemblyName, headerCode32, cppCode32, errors)
				|| !GetRpcCppCodePair(resource, rpcOutput64, assemblyName, headerCode64, cppCode64, errors))
			{
				return false;
			}

			try
			{
				auto headerCode = MergeCppMultiPlatform(headerCode32, headerCode64);
				auto cppCode = MergeCppMultiPlatform(cppCode32, cppCode64);
				return WriteRpcCppCodePairToFile(resource, assemblyName, cppFolder, headerCode, cppCode, errors);
			}
			catch (const MergeCppMultiPlatformException& ex)
			{
				AddCppGenerationError(
					resource,
					errors,
					L"Unable to merge x86/x64 RPC C++ output near x86 ("
					+ itow(ex.row32 + 1) + L"," + itow(ex.column32 + 1)
					+ L") and x64 (" + itow(ex.row64 + 1) + L"," + itow(ex.column64 + 1) + L")."
					);
				return false;
			}
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
			vint solidRows = length / block;
			vint rows = solidRows + (remain ? 1 : 0);

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
				vint size = i == solidRows ? remain : block;
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
					writer.WriteLine(L"#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA");
					writer.WriteLine(L"\t\t\t\t\tGUI_PLUGIN_DEPEND(GacUI_Instance_Reflection);");
					writer.WriteLine(L"\t\t\t\t\tGUI_PLUGIN_DEPEND(GacUI_Compiler_WorkflowTypeResolvers);");
					writer.WriteLine(L"#endif");
					writer.WriteLine(L"\t\t\t\t}");
					writer.WriteLine(L"");
					writer.WriteLine(L"\t\t\t\tvoid Load(bool controllerUnrelatedPlugins, bool controllerRelatedPlugins)override");
					writer.WriteLine(L"\t\t\t\t{");
					writer.WriteLine(L"\t\t\t\t\tif (controllerRelatedPlugins)");
					writer.WriteLine(L"\t\t\t\t\t{");
					writer.WriteLine(L"\t\t\t\t\t\tList<GuiResourceError> errors;");
					writer.WriteLine(L"\t\t\t\t\t\tMemoryStream resourceStream;");
					writer.WriteLine(L"\t\t\t\t\t\t" + cppInput->assemblyName + L"ResourceReader::ReadToStream(resourceStream);");
					writer.WriteLine(L"\t\t\t\t\t\tresourceStream.SeekFromBegin(0);");
					writer.WriteLine(L"\t\t\t\t\t\tGetResourceManager()->LoadResourceOrPending(resourceStream, GuiResourceUsage::InstanceClass);");
					writer.WriteLine(L"\t\t\t\t\t}");
					writer.WriteLine(L"\t\t\t\t}");
					writer.WriteLine(L"");
					writer.WriteLine(L"\t\t\t\tvoid Unload(bool controllerUnrelatedPlugins, bool controllerRelatedPlugins)override");
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
