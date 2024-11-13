NOTES and CAVEATS

while it's multiple .h files, you should only #include "quake2.h" internally the other includes are used. 

this is to allow other bsp formats to be added as .h files and share the obviously shared code.

-------------------------------------------------------------------------------------------------------

Fully supports loading maps and textures directly from Quake 2 pak format files. 

Fully supports loading maps and textures directly from disk 

collect your `pak0.pak` and other Quake 2 compatible PAK files and place them in the resources folder. or extract the pak into your resources folder 

-------------------------------------------------------------------------------------------------------
                        Currently the Quake 2 Remaster is NOT SUPPORTED.
-------------------------------------------------------------------------------------------------------

The lightmap data is stored a different way than the original game.

Some of the remaster maps use a slightly different BSP format for the larger levels. which is unsupported currently 

-------------------------------------------------------------------------------------------------------
            This code will ONLY correctly load Quake 2 BSP files from the ORIGINAL game. 
-------------------------------------------------------------------------------------------------------







