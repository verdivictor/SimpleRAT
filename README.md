# SimpleRAT
Remote Access Terminal (simple version) in C. 


A simple Remote Access Terminal for Windows, a client-server program which will open and listen to a socket on the server program, and connect to it from the client.

Usage:
1. Compile the server and client programs to an .exe
2. Run server.exe on the server and run the client as administrator. This will create the folder C:/Windows/Panther/setup.exe and will place the important files inside. Now you can run client.exe as a regular user, and it will connect to the server, and you can have achieved remote access, congratulations :^).
3. Have fun!

Features:
1. Only runs if inside C:/Windows/Panther/setup.exe. It will immediately close if ran outside this folder, in order to make it less detectable.
2. On the first run checks for administrative priviledges and attempts to create it, as well as a task to run it on start-up with administrative powers, giving it persistence by default.
3. Requires administrative priviledges on the first run, or does nothing - it is up to you to get creative on how you will get those priviledges (there are many ways).
4. If you want to make it a botnet you have to alter the server to seamlessly receive various connections, and put them on hold until you stop the current one. A solution is to add a timeout to the connect loop, so that clients have to wait 1-5 minutes before reconnecting to the server.
