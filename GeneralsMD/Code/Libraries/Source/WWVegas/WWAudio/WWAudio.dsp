# Microsoft Developer Studio Project File - Name="WWAudio" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=WWAudio - Win32 DebugE
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "WWAudio.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "WWAudio.mak" CFG="WWAudio - Win32 DebugE"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "WWAudio - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "WWAudio - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "WWAudio - Win32 Profile" (based on "Win32 (x86) Static Library")
!MESSAGE "WWAudio - Win32 DebugE" (based on "Win32 (x86) Static Library")
!MESSAGE "WWAudio - Win32 ProfileE" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Commando/Code/WWAudio", VIPBAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "WWAudio - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /WX /Gi /GR- /GX /O2 /Ob2 /I "..\miles6\include" /I "..\wwlib" /I "..\wwmath" /I "..\wwdebug" /I "..\ww3d2" /I "..\wwsaveload" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "WIN32" /D "G_CODE_BASE" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\Lib\WWAudio.lib"

!ELSEIF  "$(CFG)" == "WWAudio - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /WX /Gm /Gi /GR- /GX /ZI /Od /I "..\miles6\include" /I "..\wwlib" /I "..\wwmath" /I "..\wwdebug" /I "..\ww3d2" /I "..\wwsaveload" /D "_DEBUG" /D "WWDEBUG" /D "_MBCS" /D "_LIB" /D "WIN32" /D "G_CODE_BASE" /D "_WINDOWS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\Lib\WWAudioDebug.lib"

!ELSEIF  "$(CFG)" == "WWAudio - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Profile"
# PROP BASE Intermediate_Dir "Profile"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile"
# PROP Intermediate_Dir "Profile"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "..\miles" /I "..\wwlib" /I "..\wwmath" /I "..\wwdebug" /I "..\ww3d" /I "..\wwphys" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /WX /Gm /Gi /GR- /GX /Zi /O2 /Op /Ob2 /I "..\miles6\include" /I "..\wwlib" /I "..\wwmath" /I "..\wwdebug" /I "..\ww3d2" /I "..\wwsaveload" /D "NDEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "WWDEBUG" /D "_MBCS" /D "_LIB" /D "WIN32" /D "G_CODE_BASE" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\Libs\Release\WWAudio.lib"
# ADD LIB32 /nologo /out:"..\..\..\Lib\WWAudioProfile.lib"

!ELSEIF  "$(CFG)" == "WWAudio - Win32 DebugE"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DebugE"
# PROP BASE Intermediate_Dir "DebugE"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugE"
# PROP Intermediate_Dir "DebugE"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W4 /Gm /Gi /GX /ZI /Od /I "..\miles6\include" /I "..\wwlib" /I "..\wwmath" /I "..\wwdebug" /I "..\ww3d" /I "..\wwsaveload" /D "_DEBUG" /D "WWDEBUG" /D "_MBCS" /D "_LIB" /D "WIN32" /D "G_CODE_BASE" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /WX /Gm /Gi /GR- /GX /ZI /Od /I "..\miles6\include" /I "..\wwlib" /I "..\wwmath" /I "..\wwdebug" /I "..\ww3d2" /I "..\wwsaveload" /D "_DEBUG" /D "WWDEBUG" /D "_MBCS" /D "_LIB" /D "WIN32" /D "G_CODE_BASE" /D "PARAM_EDITING_ON" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\Libs\Debug\WWAudio.lib"
# ADD LIB32 /nologo /out:"..\..\..\Lib\WWAudioDebugE.lib"

!ELSEIF  "$(CFG)" == "WWAudio - Win32 ProfileE"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ProfileE"
# PROP BASE Intermediate_Dir "ProfileE"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ProfileE"
# PROP Intermediate_Dir "ProfileE"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /Gm /Gi /GX /Zi /O2 /I "..\miles6\include" /I "..\wwlib" /I "..\wwmath" /I "..\wwdebug" /I "..\ww3d" /I "..\wwsaveload" /D "NDEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "WWDEBUG" /D "_MBCS" /D "_LIB" /D "WIN32" /D "G_CODE_BASE" /YX /FD /c
# ADD CPP /nologo /MD /W3 /WX /Gm /Gi /GR- /GX /Zi /O2 /I "..\miles6\include" /I "..\wwlib" /I "..\wwmath" /I "..\wwdebug" /I "..\ww3d2" /I "..\wwsaveload" /D "NDEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "WWDEBUG" /D "_MBCS" /D "_LIB" /D "WIN32" /D "G_CODE_BASE" /D "PARAM_EDITING_ON" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\Libs\profile\WWAudio.lib"
# ADD LIB32 /nologo /out:"..\..\..\Lib\WWAudioProfileE.lib"

!ENDIF 

# Begin Target

# Name "WWAudio - Win32 Release"
# Name "WWAudio - Win32 Debug"
# Name "WWAudio - Win32 Profile"
# Name "WWAudio - Win32 DebugE"
# Name "WWAudio - Win32 ProfileE"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AudibleSound.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioSaveLoad.cpp
# End Source File
# Begin Source File

SOURCE=.\FilteredSound.cpp
# End Source File
# Begin Source File

SOURCE=.\Listener.cpp
# End Source File
# Begin Source File

SOURCE=.\listenerhandle.cpp
# End Source File
# Begin Source File

SOURCE=.\LogicalListener.cpp
# End Source File
# Begin Source File

SOURCE=.\LogicalSound.cpp
# End Source File
# Begin Source File

SOURCE=.\sound2dhandle.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound3D.cpp
# End Source File
# Begin Source File

SOURCE=.\sound3dhandle.cpp
# End Source File
# Begin Source File

SOURCE=.\SoundBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\soundhandle.cpp
# End Source File
# Begin Source File

SOURCE=.\SoundPseudo3D.cpp
# End Source File
# Begin Source File

SOURCE=.\SoundScene.cpp
# End Source File
# Begin Source File

SOURCE=.\SoundSceneObj.cpp
# End Source File
# Begin Source File

SOURCE=.\soundstreamhandle.cpp
# End Source File
# Begin Source File

SOURCE=.\Threads.cpp
# End Source File
# Begin Source File

SOURCE=.\Utils.cpp
# End Source File
# Begin Source File

SOURCE=.\WWAudio.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AABTreeSoundCullClass.h
# End Source File
# Begin Source File

SOURCE=.\AudibleSound.h
# End Source File
# Begin Source File

SOURCE=.\AudioEvents.h
# End Source File
# Begin Source File

SOURCE=.\AudioSaveLoad.h
# End Source File
# Begin Source File

SOURCE=.\FilteredSound.h
# End Source File
# Begin Source File

SOURCE=.\Listener.h
# End Source File
# Begin Source File

SOURCE=.\listenerhandle.h
# End Source File
# Begin Source File

SOURCE=.\LogicalListener.h
# End Source File
# Begin Source File

SOURCE=.\LogicalSound.h
# End Source File
# Begin Source File

SOURCE=.\PriorityVector.h
# End Source File
# Begin Source File

SOURCE=.\sound2dhandle.h
# End Source File
# Begin Source File

SOURCE=.\Sound3D.h
# End Source File
# Begin Source File

SOURCE=.\sound3dhandle.h
# End Source File
# Begin Source File

SOURCE=.\SoundBuffer.h
# End Source File
# Begin Source File

SOURCE=.\SoundCullObj.h
# End Source File
# Begin Source File

SOURCE=.\soundhandle.h
# End Source File
# Begin Source File

SOURCE=.\SoundPseudo3D.h
# End Source File
# Begin Source File

SOURCE=.\SoundScene.h
# End Source File
# Begin Source File

SOURCE=.\SoundSceneObj.h
# End Source File
# Begin Source File

SOURCE=.\soundstreamhandle.h
# End Source File
# Begin Source File

SOURCE=.\Threads.h
# End Source File
# Begin Source File

SOURCE=.\Utils.h
# End Source File
# Begin Source File

SOURCE=.\WWAudio.h
# End Source File
# End Group
# End Target
# End Project
