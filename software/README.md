# Installation of Browser Extension

Firstly enable developer mode within chrome by navigating to `chrome://extensions` and clicking on the 'Developer Mode' checkbox.

Then select the 'Load unpacked' button within the same view and select the folder `/Software/browser`. 

Finally note the extension `ID`
as this is an important parameter that is required for the installation of the native host.

# Installation of Native Host

Mark the `build.sh` and install.sh scripts as executable within a terminal and run thie build.sh script. 

A binary file named **nativehost** will be created within the
same folder. Next modify the file `com.secure.pass.json` to include the `ID` of the extension created earlier along with the full path of the nativehost binary created
earlier. 

Finally run `install.sh`.
