This project involes making a secure art gallery. There are two files to run. LogAppend adds new information to a specified log. LogRead reads information from a specified log. 

This is a secure implementation. Encryption of the log is done using the crypto++ library. Crypto++ uses AES encryption. All input validation is done using regex expressions or additional criteria. 

TO RUN:

Download the crypto++ library found here: https://github.com/weidai11/cryptopp/tree/34a34967ac560c1801bf3845dbac3ac63c1d4c05

To run on visual studio IDE: Paste the cryptopp library into the project. From [Visual Studio->Solution Explorer] click on [Solution->Add->Existing Project] and select the cryptlib.vcproj file to include in your solution. Build and run the application. Note, there are two main files here so it is likely one will have to be commented out. To add in the command line arguments go to [project -> properties -> configuration properties -> debugging -> edit the command line arguments]

To run on visual studio developers powershell: Navigate to the location where the crypto++ library was downloaded (in files). Open cryptest.sln. Build the solution in RELEASE MODE and change the target architecture to x86. After that is done building ensure there was a file in a release folder named cryptlib.lib. On the visual studio IDE go to [tools -> Command Line -> developers powershell]. This should open a new powershell window. To compile the code, run the command:

cl /EHsc logread.cpp log_utils.cpp logappend.cpp /I"path\to\cryptopp" /link /LIBPATH:"path\cryptlib\lib" cryptlib.lib

Make sure to change "path\to\cryptopp" to the location of the crypto++ library downloaded on your device. Make sure to change "path\cryptlib\lib" to the location of the cryptlib.lib file. 

You can then run a command such as ./logread -K secret -R -E Fred log1
