# Microsoft Developer Studio Project File - Name="CNCControl" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=CNCControl - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "CNCControl.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CNCControl.mak" CFG="CNCControl - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CNCControl - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "CNCControl - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CNCControl - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=fl32.exe
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0xc09 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0xc09 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=fl32.exe
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MDd /W4 /Gm /GX /ZI /Od /I "c:\derek\vc\pathmath" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0xc09 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0xc09 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /verbose /incremental:no /nodefaultlib /force

!ENDIF 

# Begin Target

# Name "CNCControl - Win32 Release"
# Name "CNCControl - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AnimationSettingsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\BoxLimit.cpp
# End Source File
# Begin Source File

SOURCE=.\ChildFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\CNCComms.cpp
# End Source File
# Begin Source File

SOURCE=.\CNCControl.rc
# End Source File
# Begin Source File

SOURCE=.\CNCControlApp.cpp
# End Source File
# Begin Source File

SOURCE=.\CNCControlDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\CNCControlView.cpp
# End Source File
# Begin Source File

SOURCE=.\CommsDataDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ContPathData.cpp
# End Source File
# Begin Source File

SOURCE=.\Controller.cpp
# End Source File
# Begin Source File

SOURCE=.\ControllerPath.cpp
# End Source File
# Begin Source File

SOURCE=.\ControllerTracker.cpp
# End Source File
# Begin Source File

SOURCE=.\DiagnoseDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\DimensionsDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\Common\DlgUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\GeneralOptionsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\GridView.cpp
# End Source File
# Begin Source File

SOURCE=.\ItemEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\LogDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\MachinePathSegDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MachinePropDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MachineSim.cpp
# End Source File
# Begin Source File

SOURCE=.\MachineState.cpp
# End Source File
# Begin Source File

SOURCE=.\MachineStatusView.cpp
# End Source File
# Begin Source File

SOURCE=.\MachineTracker.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\ManualControlView.cpp
# End Source File
# Begin Source File

SOURCE=..\Common\MoveView.cpp
# End Source File
# Begin Source File

SOURCE=.\Packet.cpp
# End Source File
# Begin Source File

SOURCE=.\Param.cpp
# End Source File
# Begin Source File

SOURCE=.\ParamDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\ParamLoader.cpp
# End Source File
# Begin Source File

SOURCE=.\ParamView.cpp
# End Source File
# Begin Source File

SOURCE=.\PathAnimateView.cpp
# End Source File
# Begin Source File

SOURCE=.\PathBuilder.cpp
# End Source File
# Begin Source File

SOURCE=.\PathDataObjects.cpp
# End Source File
# Begin Source File

SOURCE=.\PathDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\PathLoader.cpp
# End Source File
# Begin Source File

SOURCE=.\PathMove.cpp
# End Source File
# Begin Source File

SOURCE=.\PathNCIDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\PathPositionDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\PathSpeed.cpp
# End Source File
# Begin Source File

SOURCE=.\PathSpeedDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\PathSpeedView.cpp
# End Source File
# Begin Source File

SOURCE=.\PathTimeStep.cpp
# End Source File
# Begin Source File

SOURCE=.\PathTracker.cpp
# End Source File
# Begin Source File

SOURCE=.\PathView.cpp
# End Source File
# Begin Source File

SOURCE=.\PktControl.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayerBar.cpp
# End Source File
# Begin Source File

SOURCE=.\PolyCurveFit.cpp
# End Source File
# Begin Source File

SOURCE=.\PortSelDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\PosConverter.cpp
# End Source File
# Begin Source File

SOURCE=.\ProbeSampler.cpp
# End Source File
# Begin Source File

SOURCE=.\ProbeSampleView.cpp
# End Source File
# Begin Source File

SOURCE=.\ReadEPSFile.cpp
# End Source File
# Begin Source File

SOURCE=.\ReadNCIFile.cpp
# End Source File
# Begin Source File

SOURCE=.\SeekValue.cpp
# End Source File
# Begin Source File

SOURCE=.\SendBytesDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Settings.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

# ADD CPP /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SettingsData.cpp
# End Source File
# Begin Source File

SOURCE=.\SetToolDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Store.cpp
# End Source File
# Begin Source File

SOURCE=..\Common\StrUtils.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AnimationSettingsDlg.h
# End Source File
# Begin Source File

SOURCE=.\BoxLimit.h
# End Source File
# Begin Source File

SOURCE=.\ChildFrm.h
# End Source File
# Begin Source File

SOURCE=.\CNCComms.h
# End Source File
# Begin Source File

SOURCE=.\CNCControl.h
# End Source File
# Begin Source File

SOURCE=.\CNCControlApp.h
# End Source File
# Begin Source File

SOURCE=.\CNCControlDoc.h
# End Source File
# Begin Source File

SOURCE=.\CNCControlView.h
# End Source File
# Begin Source File

SOURCE=.\CommsDataDlg.h
# End Source File
# Begin Source File

SOURCE=.\ContPathData.h
# End Source File
# Begin Source File

SOURCE=.\Controller.h
# End Source File
# Begin Source File

SOURCE=.\ControllerPath.h
# End Source File
# Begin Source File

SOURCE=.\ControllerTracker.h
# End Source File
# Begin Source File

SOURCE=.\DiagnoseDlg.h
# End Source File
# Begin Source File

SOURCE=.\DimensionsDlg.h
# End Source File
# Begin Source File

SOURCE=..\Common\DlgUtils.h
# End Source File
# Begin Source File

SOURCE=..\Common\FIFOBuffer.h
# End Source File
# Begin Source File

SOURCE=.\GeneralOptionsDlg.h
# End Source File
# Begin Source File

SOURCE=.\GridView.h
# End Source File
# Begin Source File

SOURCE=.\ItemEdit.h
# End Source File
# Begin Source File

SOURCE=.\ListArray.h
# End Source File
# Begin Source File

SOURCE=.\LogDoc.h
# End Source File
# Begin Source File

SOURCE=.\MachineCodes.h
# End Source File
# Begin Source File

SOURCE=.\MachinePathSegDlg.h
# End Source File
# Begin Source File

SOURCE=.\MachinePropDlg.h
# End Source File
# Begin Source File

SOURCE=.\MachineSim.h
# End Source File
# Begin Source File

SOURCE=.\MachineState.h
# End Source File
# Begin Source File

SOURCE=.\MachineStatusView.h
# End Source File
# Begin Source File

SOURCE=.\MachineTracker.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\ManualControlView.h
# End Source File
# Begin Source File

SOURCE=..\Common\Matrix.h
# End Source File
# Begin Source File

SOURCE=..\Common\MostRecentList.h
# End Source File
# Begin Source File

SOURCE=..\Common\MoveView.h
# End Source File
# Begin Source File

SOURCE=.\Packet.h
# End Source File
# Begin Source File

SOURCE=.\Param.h
# End Source File
# Begin Source File

SOURCE=.\ParamDoc.h
# End Source File
# Begin Source File

SOURCE=.\ParamLoader.h
# End Source File
# Begin Source File

SOURCE=.\ParamView.h
# End Source File
# Begin Source File

SOURCE=.\PathAnimateView.h
# End Source File
# Begin Source File

SOURCE=.\PathBuilder.h
# End Source File
# Begin Source File

SOURCE=.\PathDataObjects.h
# End Source File
# Begin Source File

SOURCE=.\PathDoc.h
# End Source File
# Begin Source File

SOURCE=.\PathLoader.h
# End Source File
# Begin Source File

SOURCE=.\PathMove.h
# End Source File
# Begin Source File

SOURCE=.\PathNCIDoc.h
# End Source File
# Begin Source File

SOURCE=.\PathPositionDlg.h
# End Source File
# Begin Source File

SOURCE=.\PathSpeed.h
# End Source File
# Begin Source File

SOURCE=.\PathSpeedDoc.h
# End Source File
# Begin Source File

SOURCE=.\PathSpeedView.h
# End Source File
# Begin Source File

SOURCE=.\PathTimeStep.h
# End Source File
# Begin Source File

SOURCE=.\PathTracker.h
# End Source File
# Begin Source File

SOURCE=.\PathView.h
# End Source File
# Begin Source File

SOURCE=.\PktControl.h
# End Source File
# Begin Source File

SOURCE=.\PlayerBar.h
# End Source File
# Begin Source File

SOURCE=.\PolyCurveFit.h
# End Source File
# Begin Source File

SOURCE=.\PortSelDlg.h
# End Source File
# Begin Source File

SOURCE=.\PosConverter.h
# End Source File
# Begin Source File

SOURCE=.\ProbeSampler.h
# End Source File
# Begin Source File

SOURCE=.\ProbeSampleView.h
# End Source File
# Begin Source File

SOURCE=.\ReadEPSFile.h
# End Source File
# Begin Source File

SOURCE=.\ReadNCIFile.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\SeekValue.h
# End Source File
# Begin Source File

SOURCE=.\SendBytesDlg.h
# End Source File
# Begin Source File

SOURCE=.\Settings.h
# End Source File
# Begin Source File

SOURCE=.\SettingsData.h
# End Source File
# Begin Source File

SOURCE=.\SetToolDlg.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\Store.h
# End Source File
# Begin Source File

SOURCE=..\Common\StrUtils.h
# End Source File
# Begin Source File

SOURCE=.\ThreadMessages.h
# End Source File
# Begin Source File

SOURCE=..\Common\Vector.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bitmap2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bitmap3.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00001.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00002.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00003.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00004.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00005.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00006.bmp
# End Source File
# Begin Source File

SOURCE=.\res\buttondo.bmp
# End Source File
# Begin Source File

SOURCE=.\res\buttonup.bmp
# End Source File
# Begin Source File

SOURCE=.\res\CNCControl.ico
# End Source File
# Begin Source File

SOURCE=.\res\CNCControl.rc2
# End Source File
# Begin Source File

SOURCE=.\res\CNCControlDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\idc_butt.bmp
# End Source File
# Begin Source File

SOURCE=.\res\motionpl.bmp
# End Source File
# Begin Source File

SOURCE=.\res\pathtoolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\playerba.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\toolbar1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\toolbarz.bmp
# End Source File
# Begin Source File

SOURCE=.\res\viewdirb.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\Cnc.par
# End Source File
# Begin Source File

SOURCE=.\CNCControl.set
# End Source File
# Begin Source File

SOURCE=.\Commands.txt
# End Source File
# Begin Source File

SOURCE=".\Formula CNCControl.txt"
# End Source File
# Begin Source File

SOURCE=".\Formula Curve Fit Splitting.txt"
# End Source File
# Begin Source File

SOURCE=".\Formula Motor Control.txt"
# End Source File
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# Begin Source File

SOURCE=".\Results Limits.txt"
# End Source File
# Begin Source File

SOURCE=".\Results S.txt"
# End Source File
# Begin Source File

SOURCE=".\Results Time.txt"
# End Source File
# Begin Source File

SOURCE=.\Software.txt
# End Source File
# Begin Source File

SOURCE=.\ToDo.txt
# End Source File
# Begin Source File

SOURCE=..\PathMath\Debug\PathMath.lib
# End Source File
# End Target
# End Project
