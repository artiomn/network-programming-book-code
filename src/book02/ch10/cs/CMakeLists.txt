# Simple build command: `mcs -out:echo-server.exe`

project("${CHAPTER_NAME}cs-async-echo-server" CSharp)

add_executable("${PROJECT}" echo-server.cs)
