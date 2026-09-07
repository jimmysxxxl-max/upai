UniquePlayerRedirector 0.2.5 hands/face test

For your existing flattened GitHub repo:
1. Extract UniquePlayerRedirector_0.2.5_GITHUB_DROP.zip.
2. Upload/replace every file from that ZIP at the ROOT of your repository.
3. Commit the changes.
4. Do NOT change the working GitHub Actions workflow.
5. Run Build UniquePlayerRedirector DLL again.

What changed from 0.2.4:
- Fixed the current CommonLibF4 logging compile failure.
- Removed the unavailable F4SE::log::log_directory() call.
- Included the full spdlog API header.
- Routed all UPR diagnostic messages through the dedicated spdlog file logger.
- Diagnostic log now writes to Data\F4SE\Plugins\UniquePlayerRedirector.log.
- Keeps the 0.2.4 hand/face routing changes and targeted rebuild logic.

After testing, if hands or face still fail, send:
Data\F4SE\Plugins\UniquePlayerRedirector.log
