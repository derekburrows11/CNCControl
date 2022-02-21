# Microsoft Developer Studio Generated NMAKE File, Based on CNCControl.dsp
!IF "$(CFG)" == ""
CFG=CNCControl - Win32 Debug
!MESSAGE No configuration specified. Defaulting to CNCControl - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "CNCControl - Win32 Release" && "$(CFG)" !=\
 "CNCControl - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "CNCControl - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\CNCControl.exe"

!ELSE 

ALL : "PathMath - Win32 Release" "$(OUTDIR)\CNCControl.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"PathMath - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\AnimationSettingsDlg.obj"
	-@erase "$(INTDIR)\ChildFrm.obj"
	-@erase "$(INTDIR)\CNCComms.obj"
	-@erase "$(INTDIR)\CNCControl.pch"
	-@erase "$(INTDIR)\CNCControl.res"
	-@erase "$(INTDIR)\CNCControlApp.obj"
	-@erase "$(INTDIR)\CNCControlDoc.obj"
	-@erase "$(INTDIR)\CNCControlView.obj"
	-@erase "$(INTDIR)\CommsDataDlg.obj"
	-@erase "$(INTDIR)\ContPathData.obj"
	-@erase "$(INTDIR)\Controller.obj"
	-@erase "$(INTDIR)\ControllerPath.obj"
	-@erase "$(INTDIR)\ControllerTracker.obj"
	-@erase "$(INTDIR)\ControlStatusDlg.obj"
	-@erase "$(INTDIR)\DiagnoseDlg.obj"
	-@erase "$(INTDIR)\GridView.obj"
	-@erase "$(INTDIR)\ItemEdit.obj"
	-@erase "$(INTDIR)\MachinePathSegDlg.obj"
	-@erase "$(INTDIR)\MachinePropDlg.obj"
	-@erase "$(INTDIR)\MachineSim.obj"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\MoveView.obj"
	-@erase "$(INTDIR)\Packet.obj"
	-@erase "$(INTDIR)\Param.obj"
	-@erase "$(INTDIR)\ParamDoc.obj"
	-@erase "$(INTDIR)\ParamLoader.obj"
	-@erase "$(INTDIR)\ParamView.obj"
	-@erase "$(INTDIR)\PathAnimateView.obj"
	-@erase "$(INTDIR)\PathDataObjects.obj"
	-@erase "$(INTDIR)\PathDoc.obj"
	-@erase "$(INTDIR)\PathLoader.obj"
	-@erase "$(INTDIR)\PathMove.obj"
	-@erase "$(INTDIR)\PathNCIDoc.obj"
	-@erase "$(INTDIR)\PathPositionDlg.obj"
	-@erase "$(INTDIR)\PathSpeed.obj"
	-@erase "$(INTDIR)\PathSpeedDoc.obj"
	-@erase "$(INTDIR)\PathSpeedView.obj"
	-@erase "$(INTDIR)\PathTimeStep.obj"
	-@erase "$(INTDIR)\PathTracker.obj"
	-@erase "$(INTDIR)\PathView.obj"
	-@erase "$(INTDIR)\PktControl.obj"
	-@erase "$(INTDIR)\PlayerBar.obj"
	-@erase "$(INTDIR)\PortSelDlg.obj"
	-@erase "$(INTDIR)\ReadNCIFile.obj"
	-@erase "$(INTDIR)\SeekValue.obj"
	-@erase "$(INTDIR)\Settings.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StrUtils.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(OUTDIR)\CNCControl.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

F90=fl32.exe
CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D\
 "_AFXDLL" /Fp"$(INTDIR)\CNCControl.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Release/
CPP_SBRS=.

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o NUL /win32 
RSC=rc.exe
RSC_PROJ=/l 0xc09 /fo"$(INTDIR)\CNCControl.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\CNCControl.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)\CNCControl.pdb" /machine:I386 /out:"$(OUTDIR)\CNCControl.exe" 
LINK32_OBJS= \
	"$(INTDIR)\AnimationSettingsDlg.obj" \
	"$(INTDIR)\ChildFrm.obj" \
	"$(INTDIR)\CNCComms.obj" \
	"$(INTDIR)\CNCControl.res" \
	"$(INTDIR)\CNCControlApp.obj" \
	"$(INTDIR)\CNCControlDoc.obj" \
	"$(INTDIR)\CNCControlView.obj" \
	"$(INTDIR)\CommsDataDlg.obj" \
	"$(INTDIR)\ContPathData.obj" \
	"$(INTDIR)\Controller.obj" \
	"$(INTDIR)\ControllerPath.obj" \
	"$(INTDIR)\ControllerTracker.obj" \
	"$(INTDIR)\ControlStatusDlg.obj" \
	"$(INTDIR)\DiagnoseDlg.obj" \
	"$(INTDIR)\GridView.obj" \
	"$(INTDIR)\ItemEdit.obj" \
	"$(INTDIR)\MachinePathSegDlg.obj" \
	"$(INTDIR)\MachinePropDlg.obj" \
	"$(INTDIR)\MachineSim.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\MoveView.obj" \
	"$(INTDIR)\Packet.obj" \
	"$(INTDIR)\Param.obj" \
	"$(INTDIR)\ParamDoc.obj" \
	"$(INTDIR)\ParamLoader.obj" \
	"$(INTDIR)\ParamView.obj" \
	"$(INTDIR)\PathAnimateView.obj" \
	"$(INTDIR)\PathDataObjects.obj" \
	"$(INTDIR)\PathDoc.obj" \
	"$(INTDIR)\PathLoader.obj" \
	"$(INTDIR)\PathMove.obj" \
	"$(INTDIR)\PathNCIDoc.obj" \
	"$(INTDIR)\PathPositionDlg.obj" \
	"$(INTDIR)\PathSpeed.obj" \
	"$(INTDIR)\PathSpeedDoc.obj" \
	"$(INTDIR)\PathSpeedView.obj" \
	"$(INTDIR)\PathTimeStep.obj" \
	"$(INTDIR)\PathTracker.obj" \
	"$(INTDIR)\PathView.obj" \
	"$(INTDIR)\PktControl.obj" \
	"$(INTDIR)\PlayerBar.obj" \
	"$(INTDIR)\PortSelDlg.obj" \
	"$(INTDIR)\ReadNCIFile.obj" \
	"$(INTDIR)\SeekValue.obj" \
	"$(INTDIR)\Settings.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\StrUtils.obj" \
	"..\PathMath\Debug\PathMath.lib" \
	"..\PathMath\Release\PathMath.lib"

"$(OUTDIR)\CNCControl.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\CNCControl.exe" "$(OUTDIR)\CNCControl.bsc"

!ELSE 

ALL : "PathMath - Win32 Debug" "$(OUTDIR)\CNCControl.exe"\
 "$(OUTDIR)\CNCControl.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"PathMath - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\AnimationSettingsDlg.obj"
	-@erase "$(INTDIR)\AnimationSettingsDlg.sbr"
	-@erase "$(INTDIR)\ChildFrm.obj"
	-@erase "$(INTDIR)\ChildFrm.sbr"
	-@erase "$(INTDIR)\CNCComms.obj"
	-@erase "$(INTDIR)\CNCComms.sbr"
	-@erase "$(INTDIR)\CNCControl.pch"
	-@erase "$(INTDIR)\CNCControl.res"
	-@erase "$(INTDIR)\CNCControlApp.obj"
	-@erase "$(INTDIR)\CNCControlApp.sbr"
	-@erase "$(INTDIR)\CNCControlDoc.obj"
	-@erase "$(INTDIR)\CNCControlDoc.sbr"
	-@erase "$(INTDIR)\CNCControlView.obj"
	-@erase "$(INTDIR)\CNCControlView.sbr"
	-@erase "$(INTDIR)\CommsDataDlg.obj"
	-@erase "$(INTDIR)\CommsDataDlg.sbr"
	-@erase "$(INTDIR)\ContPathData.obj"
	-@erase "$(INTDIR)\ContPathData.sbr"
	-@erase "$(INTDIR)\Controller.obj"
	-@erase "$(INTDIR)\Controller.sbr"
	-@erase "$(INTDIR)\ControllerPath.obj"
	-@erase "$(INTDIR)\ControllerPath.sbr"
	-@erase "$(INTDIR)\ControllerTracker.obj"
	-@erase "$(INTDIR)\ControllerTracker.sbr"
	-@erase "$(INTDIR)\ControlStatusDlg.obj"
	-@erase "$(INTDIR)\ControlStatusDlg.sbr"
	-@erase "$(INTDIR)\DiagnoseDlg.obj"
	-@erase "$(INTDIR)\DiagnoseDlg.sbr"
	-@erase "$(INTDIR)\GridView.obj"
	-@erase "$(INTDIR)\GridView.sbr"
	-@erase "$(INTDIR)\ItemEdit.obj"
	-@erase "$(INTDIR)\ItemEdit.sbr"
	-@erase "$(INTDIR)\MachinePathSegDlg.obj"
	-@erase "$(INTDIR)\MachinePathSegDlg.sbr"
	-@erase "$(INTDIR)\MachinePropDlg.obj"
	-@erase "$(INTDIR)\MachinePropDlg.sbr"
	-@erase "$(INTDIR)\MachineSim.obj"
	-@erase "$(INTDIR)\MachineSim.sbr"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\MainFrm.sbr"
	-@erase "$(INTDIR)\MoveView.obj"
	-@erase "$(INTDIR)\MoveView.sbr"
	-@erase "$(INTDIR)\Packet.obj"
	-@erase "$(INTDIR)\Packet.sbr"
	-@erase "$(INTDIR)\Param.obj"
	-@erase "$(INTDIR)\Param.sbr"
	-@erase "$(INTDIR)\ParamDoc.obj"
	-@erase "$(INTDIR)\ParamDoc.sbr"
	-@erase "$(INTDIR)\ParamLoader.obj"
	-@erase "$(INTDIR)\ParamLoader.sbr"
	-@erase "$(INTDIR)\ParamView.obj"
	-@erase "$(INTDIR)\ParamView.sbr"
	-@erase "$(INTDIR)\PathAnimateView.obj"
	-@erase "$(INTDIR)\PathAnimateView.sbr"
	-@erase "$(INTDIR)\PathDataObjects.obj"
	-@erase "$(INTDIR)\PathDataObjects.sbr"
	-@erase "$(INTDIR)\PathDoc.obj"
	-@erase "$(INTDIR)\PathDoc.sbr"
	-@erase "$(INTDIR)\PathLoader.obj"
	-@erase "$(INTDIR)\PathLoader.sbr"
	-@erase "$(INTDIR)\PathMove.obj"
	-@erase "$(INTDIR)\PathMove.sbr"
	-@erase "$(INTDIR)\PathNCIDoc.obj"
	-@erase "$(INTDIR)\PathNCIDoc.sbr"
	-@erase "$(INTDIR)\PathPositionDlg.obj"
	-@erase "$(INTDIR)\PathPositionDlg.sbr"
	-@erase "$(INTDIR)\PathSpeed.obj"
	-@erase "$(INTDIR)\PathSpeed.sbr"
	-@erase "$(INTDIR)\PathSpeedDoc.obj"
	-@erase "$(INTDIR)\PathSpeedDoc.sbr"
	-@erase "$(INTDIR)\PathSpeedView.obj"
	-@erase "$(INTDIR)\PathSpeedView.sbr"
	-@erase "$(INTDIR)\PathTimeStep.obj"
	-@erase "$(INTDIR)\PathTimeStep.sbr"
	-@erase "$(INTDIR)\PathTracker.obj"
	-@erase "$(INTDIR)\PathTracker.sbr"
	-@erase "$(INTDIR)\PathView.obj"
	-@erase "$(INTDIR)\PathView.sbr"
	-@erase "$(INTDIR)\PktControl.obj"
	-@erase "$(INTDIR)\PktControl.sbr"
	-@erase "$(INTDIR)\PlayerBar.obj"
	-@erase "$(INTDIR)\PlayerBar.sbr"
	-@erase "$(INTDIR)\PortSelDlg.obj"
	-@erase "$(INTDIR)\PortSelDlg.sbr"
	-@erase "$(INTDIR)\ReadNCIFile.obj"
	-@erase "$(INTDIR)\ReadNCIFile.sbr"
	-@erase "$(INTDIR)\SeekValue.obj"
	-@erase "$(INTDIR)\SeekValue.sbr"
	-@erase "$(INTDIR)\Settings.obj"
	-@erase "$(INTDIR)\Settings.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\StrUtils.obj"
	-@erase "$(INTDIR)\StrUtils.sbr"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(OUTDIR)\CNCControl.bsc"
	-@erase "$(OUTDIR)\CNCControl.exe"
	-@erase "$(OUTDIR)\CNCControl.ilk"
	-@erase "$(OUTDIR)\CNCControl.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

F90=fl32.exe
CPP=cl.exe
CPP_PROJ=/nologo /MDd /W4 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /D "_AFXDLL" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\CNCControl.pch" /Yu"stdafx.h"\
 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\Debug/

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o NUL /win32 
RSC=rc.exe
RSC_PROJ=/l 0xc09 /fo"$(INTDIR)\CNCControl.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\CNCControl.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\AnimationSettingsDlg.sbr" \
	"$(INTDIR)\ChildFrm.sbr" \
	"$(INTDIR)\CNCComms.sbr" \
	"$(INTDIR)\CNCControlApp.sbr" \
	"$(INTDIR)\CNCControlDoc.sbr" \
	"$(INTDIR)\CNCControlView.sbr" \
	"$(INTDIR)\CommsDataDlg.sbr" \
	"$(INTDIR)\ContPathData.sbr" \
	"$(INTDIR)\Controller.sbr" \
	"$(INTDIR)\ControllerPath.sbr" \
	"$(INTDIR)\ControllerTracker.sbr" \
	"$(INTDIR)\ControlStatusDlg.sbr" \
	"$(INTDIR)\DiagnoseDlg.sbr" \
	"$(INTDIR)\GridView.sbr" \
	"$(INTDIR)\ItemEdit.sbr" \
	"$(INTDIR)\MachinePathSegDlg.sbr" \
	"$(INTDIR)\MachinePropDlg.sbr" \
	"$(INTDIR)\MachineSim.sbr" \
	"$(INTDIR)\MainFrm.sbr" \
	"$(INTDIR)\MoveView.sbr" \
	"$(INTDIR)\Packet.sbr" \
	"$(INTDIR)\Param.sbr" \
	"$(INTDIR)\ParamDoc.sbr" \
	"$(INTDIR)\ParamLoader.sbr" \
	"$(INTDIR)\ParamView.sbr" \
	"$(INTDIR)\PathAnimateView.sbr" \
	"$(INTDIR)\PathDataObjects.sbr" \
	"$(INTDIR)\PathDoc.sbr" \
	"$(INTDIR)\PathLoader.sbr" \
	"$(INTDIR)\PathMove.sbr" \
	"$(INTDIR)\PathNCIDoc.sbr" \
	"$(INTDIR)\PathPositionDlg.sbr" \
	"$(INTDIR)\PathSpeed.sbr" \
	"$(INTDIR)\PathSpeedDoc.sbr" \
	"$(INTDIR)\PathSpeedView.sbr" \
	"$(INTDIR)\PathTimeStep.sbr" \
	"$(INTDIR)\PathTracker.sbr" \
	"$(INTDIR)\PathView.sbr" \
	"$(INTDIR)\PktControl.sbr" \
	"$(INTDIR)\PlayerBar.sbr" \
	"$(INTDIR)\PortSelDlg.sbr" \
	"$(INTDIR)\ReadNCIFile.sbr" \
	"$(INTDIR)\SeekValue.sbr" \
	"$(INTDIR)\Settings.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\StrUtils.sbr"

"$(OUTDIR)\CNCControl.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)\CNCControl.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)\CNCControl.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\AnimationSettingsDlg.obj" \
	"$(INTDIR)\ChildFrm.obj" \
	"$(INTDIR)\CNCComms.obj" \
	"$(INTDIR)\CNCControl.res" \
	"$(INTDIR)\CNCControlApp.obj" \
	"$(INTDIR)\CNCControlDoc.obj" \
	"$(INTDIR)\CNCControlView.obj" \
	"$(INTDIR)\CommsDataDlg.obj" \
	"$(INTDIR)\ContPathData.obj" \
	"$(INTDIR)\Controller.obj" \
	"$(INTDIR)\ControllerPath.obj" \
	"$(INTDIR)\ControllerTracker.obj" \
	"$(INTDIR)\ControlStatusDlg.obj" \
	"$(INTDIR)\DiagnoseDlg.obj" \
	"$(INTDIR)\GridView.obj" \
	"$(INTDIR)\ItemEdit.obj" \
	"$(INTDIR)\MachinePathSegDlg.obj" \
	"$(INTDIR)\MachinePropDlg.obj" \
	"$(INTDIR)\MachineSim.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\MoveView.obj" \
	"$(INTDIR)\Packet.obj" \
	"$(INTDIR)\Param.obj" \
	"$(INTDIR)\ParamDoc.obj" \
	"$(INTDIR)\ParamLoader.obj" \
	"$(INTDIR)\ParamView.obj" \
	"$(INTDIR)\PathAnimateView.obj" \
	"$(INTDIR)\PathDataObjects.obj" \
	"$(INTDIR)\PathDoc.obj" \
	"$(INTDIR)\PathLoader.obj" \
	"$(INTDIR)\PathMove.obj" \
	"$(INTDIR)\PathNCIDoc.obj" \
	"$(INTDIR)\PathPositionDlg.obj" \
	"$(INTDIR)\PathSpeed.obj" \
	"$(INTDIR)\PathSpeedDoc.obj" \
	"$(INTDIR)\PathSpeedView.obj" \
	"$(INTDIR)\PathTimeStep.obj" \
	"$(INTDIR)\PathTracker.obj" \
	"$(INTDIR)\PathView.obj" \
	"$(INTDIR)\PktControl.obj" \
	"$(INTDIR)\PlayerBar.obj" \
	"$(INTDIR)\PortSelDlg.obj" \
	"$(INTDIR)\ReadNCIFile.obj" \
	"$(INTDIR)\SeekValue.obj" \
	"$(INTDIR)\Settings.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\StrUtils.obj" \
	"..\PathMath\Debug\PathMath.lib"

"$(OUTDIR)\CNCControl.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "CNCControl - Win32 Release" || "$(CFG)" ==\
 "CNCControl - Win32 Debug"
SOURCE=.\AnimationSettingsDlg.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_ANIMA=\
	".\AnimationSettingsDlg.h"\
	".\CNCControl.h"\
	".\StdAfx.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	

"$(INTDIR)\AnimationSettingsDlg.obj" : $(SOURCE) $(DEP_CPP_ANIMA) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_ANIMA=\
	".\AnimationSettingsDlg.h"\
	".\CNCControl.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	

"$(INTDIR)\AnimationSettingsDlg.obj"	"$(INTDIR)\AnimationSettingsDlg.sbr" : \
$(SOURCE) $(DEP_CPP_ANIMA) "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\ChildFrm.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_CHILD=\
	".\ChildFrm.h"\
	".\CNCControl.h"\
	".\StdAfx.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	

"$(INTDIR)\ChildFrm.obj" : $(SOURCE) $(DEP_CPP_CHILD) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_CHILD=\
	".\ChildFrm.h"\
	".\CNCControl.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	

"$(INTDIR)\ChildFrm.obj"	"$(INTDIR)\ChildFrm.sbr" : $(SOURCE) $(DEP_CPP_CHILD)\
 "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\CNCComms.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_CNCCO=\
	".\CNCComms.h"\
	".\CNCControl.h"\
	".\CNCControlApp.h"\
	".\CommsDataDlg.h"\
	".\ContPathData.h"\
	".\Controller.h"\
	".\ControllerPath.h"\
	".\ControllerTracker.h"\
	".\ControlStatusDlg.h"\
	".\MachineSim.h"\
	".\Packet.h"\
	".\Param.h"\
	".\PathDataObjects.h"\
	".\PathDoc.h"\
	".\PathMove.h"\
	".\PathSpeed.h"\
	".\PathTimeStep.h"\
	".\PathTracker.h"\
	".\PktControl.h"\
	".\PortSelDlg.h"\
	".\SeekValue.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"PointSumInfo.h"\
	{$(INCLUDE)}"PolySegFit.h"\
	{$(INCLUDE)}"PolySegInfo.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\CNCComms.obj" : $(SOURCE) $(DEP_CPP_CNCCO) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_CNCCO=\
	".\CNCComms.h"\
	".\CNCControl.h"\
	".\CNCControlApp.h"\
	".\CommsDataDlg.h"\
	".\ContPathData.h"\
	".\Controller.h"\
	".\ControllerPath.h"\
	".\ControllerTracker.h"\
	".\ControlStatusDlg.h"\
	".\MachineSim.h"\
	".\Packet.h"\
	".\Param.h"\
	".\PathDataObjects.h"\
	".\PathDoc.h"\
	".\PathMove.h"\
	".\PathSpeed.h"\
	".\PathTimeStep.h"\
	".\PathTracker.h"\
	".\PktControl.h"\
	".\PortSelDlg.h"\
	".\SeekValue.h"\
	".\Settings.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"PointSumInfo.h"\
	{$(INCLUDE)}"PolySegFit.h"\
	{$(INCLUDE)}"PolySegInfo.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\CNCComms.obj"	"$(INTDIR)\CNCComms.sbr" : $(SOURCE) $(DEP_CPP_CNCCO)\
 "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\CNCControl.rc
DEP_RSC_CNCCON=\
	".\res\bitmap1.bmp"\
	".\res\bitmap2.bmp"\
	".\res\CNCControl.ico"\
	".\res\CNCControl.rc2"\
	".\res\CNCControlDoc.ico"\
	".\res\motionpl.bmp"\
	".\res\pathtoolbar.bmp"\
	".\res\playerba.bmp"\
	".\res\Toolbar.bmp"\
	".\res\toolbarz.bmp"\
	".\res\viewdirb.bmp"\
	

"$(INTDIR)\CNCControl.res" : $(SOURCE) $(DEP_RSC_CNCCON) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\CNCControlApp.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_CNCCONT=\
	"..\graph\plot.h"\
	".\ChildFrm.h"\
	".\CNCComms.h"\
	".\CNCControl.h"\
	".\CNCControlApp.h"\
	".\CNCControlDoc.h"\
	".\CNCControlView.h"\
	".\CommsDataDlg.h"\
	".\ContPathData.h"\
	".\Controller.h"\
	".\ControllerPath.h"\
	".\ControllerTracker.h"\
	".\ControlStatusDlg.h"\
	".\DiagnoseDlg.h"\
	".\GridView.h"\
	".\MachineSim.h"\
	".\MainFrm.h"\
	".\Packet.h"\
	".\Param.h"\
	".\ParamDoc.h"\
	".\ParamLoader.h"\
	".\ParamView.h"\
	".\PathAnimateView.h"\
	".\PathDataObjects.h"\
	".\PathDoc.h"\
	".\PathMove.h"\
	".\PathNCIDoc.h"\
	".\PathPositionDlg.h"\
	".\PathSpeed.h"\
	".\PathSpeedDoc.h"\
	".\PathSpeedView.h"\
	".\PathTimeStep.h"\
	".\PathTracker.h"\
	".\PathView.h"\
	".\PktControl.h"\
	".\PlayerBar.h"\
	".\SeekValue.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\StrUtils.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"colors.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"MoveView.h"\
	{$(INCLUDE)}"PointSumInfo.h"\
	{$(INCLUDE)}"PolySegFit.h"\
	{$(INCLUDE)}"PolySegInfo.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\CNCControlApp.obj" : $(SOURCE) $(DEP_CPP_CNCCONT) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_CNCCONT=\
	"..\graph\plot.h"\
	".\ChildFrm.h"\
	".\CNCComms.h"\
	".\CNCControl.h"\
	".\CNCControlApp.h"\
	".\CNCControlDoc.h"\
	".\CNCControlView.h"\
	".\CommsDataDlg.h"\
	".\ContPathData.h"\
	".\Controller.h"\
	".\ControllerPath.h"\
	".\ControllerTracker.h"\
	".\ControlStatusDlg.h"\
	".\DiagnoseDlg.h"\
	".\GridView.h"\
	".\MachineSim.h"\
	".\MainFrm.h"\
	".\Packet.h"\
	".\Param.h"\
	".\ParamDoc.h"\
	".\ParamLoader.h"\
	".\ParamView.h"\
	".\PathAnimateView.h"\
	".\PathDataObjects.h"\
	".\PathDoc.h"\
	".\PathMove.h"\
	".\PathNCIDoc.h"\
	".\PathPositionDlg.h"\
	".\PathSpeed.h"\
	".\PathSpeedDoc.h"\
	".\PathSpeedView.h"\
	".\PathTimeStep.h"\
	".\PathTracker.h"\
	".\PathView.h"\
	".\PktControl.h"\
	".\PlayerBar.h"\
	".\SeekValue.h"\
	".\Settings.h"\
	".\StrUtils.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"colors.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"MoveView.h"\
	{$(INCLUDE)}"PointSumInfo.h"\
	{$(INCLUDE)}"PolySegFit.h"\
	{$(INCLUDE)}"PolySegInfo.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\CNCControlApp.obj"	"$(INTDIR)\CNCControlApp.sbr" : $(SOURCE)\
 $(DEP_CPP_CNCCONT) "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\CNCControlDoc.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_CNCCONTR=\
	".\CNCControl.h"\
	".\CNCControlDoc.h"\
	".\StdAfx.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	

"$(INTDIR)\CNCControlDoc.obj" : $(SOURCE) $(DEP_CPP_CNCCONTR) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_CNCCONTR=\
	".\CNCControl.h"\
	".\CNCControlDoc.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	

"$(INTDIR)\CNCControlDoc.obj"	"$(INTDIR)\CNCControlDoc.sbr" : $(SOURCE)\
 $(DEP_CPP_CNCCONTR) "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\CNCControlView.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_CNCCONTRO=\
	".\CNCControl.h"\
	".\CNCControlDoc.h"\
	".\CNCControlView.h"\
	".\StdAfx.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	

"$(INTDIR)\CNCControlView.obj" : $(SOURCE) $(DEP_CPP_CNCCONTRO) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_CNCCONTRO=\
	".\CNCControl.h"\
	".\CNCControlDoc.h"\
	".\CNCControlView.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	

"$(INTDIR)\CNCControlView.obj"	"$(INTDIR)\CNCControlView.sbr" : $(SOURCE)\
 $(DEP_CPP_CNCCONTRO) "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\CommsDataDlg.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_COMMS=\
	".\CNCControl.h"\
	".\CommsDataDlg.h"\
	".\StdAfx.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	

"$(INTDIR)\CommsDataDlg.obj" : $(SOURCE) $(DEP_CPP_COMMS) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_COMMS=\
	".\CNCControl.h"\
	".\CommsDataDlg.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	

"$(INTDIR)\CommsDataDlg.obj"	"$(INTDIR)\CommsDataDlg.sbr" : $(SOURCE)\
 $(DEP_CPP_COMMS) "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\ContPathData.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_CONTP=\
	".\CNCControl.h"\
	".\ContPathData.h"\
	".\ControllerTracker.h"\
	".\Param.h"\
	".\PathDataObjects.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\ContPathData.obj" : $(SOURCE) $(DEP_CPP_CONTP) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_CONTP=\
	".\CNCControl.h"\
	".\ContPathData.h"\
	".\ControllerTracker.h"\
	".\Param.h"\
	".\PathDataObjects.h"\
	".\Settings.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\ContPathData.obj"	"$(INTDIR)\ContPathData.sbr" : $(SOURCE)\
 $(DEP_CPP_CONTP) "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\Controller.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_CONTR=\
	".\CNCControl.h"\
	".\ContPathData.h"\
	".\Controller.h"\
	".\ControllerTracker.h"\
	".\MachineSim.h"\
	".\Packet.h"\
	".\Param.h"\
	".\ParamDoc.h"\
	".\PathDataObjects.h"\
	".\PktControl.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\Controller.obj" : $(SOURCE) $(DEP_CPP_CONTR) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_CONTR=\
	".\CNCControl.h"\
	".\ContPathData.h"\
	".\Controller.h"\
	".\ControllerTracker.h"\
	".\MachineSim.h"\
	".\Packet.h"\
	".\Param.h"\
	".\ParamDoc.h"\
	".\PathDataObjects.h"\
	".\PktControl.h"\
	".\Settings.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\Controller.obj"	"$(INTDIR)\Controller.sbr" : $(SOURCE)\
 $(DEP_CPP_CONTR) "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\ControllerPath.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_CONTRO=\
	".\CNCComms.h"\
	".\CNCControl.h"\
	".\CommsDataDlg.h"\
	".\ContPathData.h"\
	".\Controller.h"\
	".\ControllerPath.h"\
	".\ControllerTracker.h"\
	".\ControlStatusDlg.h"\
	".\MachineSim.h"\
	".\Packet.h"\
	".\Param.h"\
	".\PathDataObjects.h"\
	".\PathDoc.h"\
	".\PathMove.h"\
	".\PathSpeed.h"\
	".\PathTimeStep.h"\
	".\PathTracker.h"\
	".\PktControl.h"\
	".\SeekValue.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"PointSumInfo.h"\
	{$(INCLUDE)}"PolySegFit.h"\
	{$(INCLUDE)}"PolySegInfo.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\ControllerPath.obj" : $(SOURCE) $(DEP_CPP_CONTRO) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_CONTRO=\
	".\CNCComms.h"\
	".\CNCControl.h"\
	".\CommsDataDlg.h"\
	".\ContPathData.h"\
	".\Controller.h"\
	".\ControllerPath.h"\
	".\ControllerTracker.h"\
	".\ControlStatusDlg.h"\
	".\MachineSim.h"\
	".\Packet.h"\
	".\Param.h"\
	".\PathDataObjects.h"\
	".\PathDoc.h"\
	".\PathMove.h"\
	".\PathSpeed.h"\
	".\PathTimeStep.h"\
	".\PathTracker.h"\
	".\PktControl.h"\
	".\SeekValue.h"\
	".\Settings.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"PointSumInfo.h"\
	{$(INCLUDE)}"PolySegFit.h"\
	{$(INCLUDE)}"PolySegInfo.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\ControllerPath.obj"	"$(INTDIR)\ControllerPath.sbr" : $(SOURCE)\
 $(DEP_CPP_CONTRO) "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\ControllerTracker.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_CONTROL=\
	"..\graph\plot.h"\
	".\ControllerTracker.h"\
	".\PathDataObjects.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"colors.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\ControllerTracker.obj" : $(SOURCE) $(DEP_CPP_CONTROL) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_CONTROL=\
	"..\graph\plot.h"\
	".\ControllerTracker.h"\
	".\PathDataObjects.h"\
	".\Settings.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"colors.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\ControllerTracker.obj"	"$(INTDIR)\ControllerTracker.sbr" : $(SOURCE)\
 $(DEP_CPP_CONTROL) "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\ControlStatusDlg.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_CONTROLS=\
	".\CNCControl.h"\
	".\ControlStatusDlg.h"\
	".\PathDataObjects.h"\
	".\StdAfx.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\ControlStatusDlg.obj" : $(SOURCE) $(DEP_CPP_CONTROLS) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_CONTROLS=\
	".\CNCControl.h"\
	".\ControlStatusDlg.h"\
	".\PathDataObjects.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\ControlStatusDlg.obj"	"$(INTDIR)\ControlStatusDlg.sbr" : $(SOURCE)\
 $(DEP_CPP_CONTROLS) "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\DiagnoseDlg.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_DIAGN=\
	"..\graph\plot.h"\
	".\CNCComms.h"\
	".\CNCControl.h"\
	".\CommsDataDlg.h"\
	".\ContPathData.h"\
	".\Controller.h"\
	".\ControllerTracker.h"\
	".\DiagnoseDlg.h"\
	".\MachineSim.h"\
	".\MainFrm.h"\
	".\Packet.h"\
	".\Param.h"\
	".\PathDataObjects.h"\
	".\PktControl.h"\
	".\PlayerBar.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"colors.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\DiagnoseDlg.obj" : $(SOURCE) $(DEP_CPP_DIAGN) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_DIAGN=\
	"..\graph\plot.h"\
	".\CNCComms.h"\
	".\CNCControl.h"\
	".\CommsDataDlg.h"\
	".\ContPathData.h"\
	".\Controller.h"\
	".\ControllerTracker.h"\
	".\DiagnoseDlg.h"\
	".\MachineSim.h"\
	".\MainFrm.h"\
	".\Packet.h"\
	".\Param.h"\
	".\PathDataObjects.h"\
	".\PktControl.h"\
	".\PlayerBar.h"\
	".\Settings.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"colors.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\DiagnoseDlg.obj"	"$(INTDIR)\DiagnoseDlg.sbr" : $(SOURCE)\
 $(DEP_CPP_DIAGN) "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\GridView.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_GRIDV=\
	".\CNCControl.h"\
	".\GridView.h"\
	".\ItemEdit.h"\
	".\StdAfx.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	

"$(INTDIR)\GridView.obj" : $(SOURCE) $(DEP_CPP_GRIDV) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_GRIDV=\
	".\CNCControl.h"\
	".\GridView.h"\
	".\ItemEdit.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	

"$(INTDIR)\GridView.obj"	"$(INTDIR)\GridView.sbr" : $(SOURCE) $(DEP_CPP_GRIDV)\
 "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\ItemEdit.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_ITEME=\
	".\CNCControl.h"\
	".\ItemEdit.h"\
	".\StdAfx.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	

"$(INTDIR)\ItemEdit.obj" : $(SOURCE) $(DEP_CPP_ITEME) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_ITEME=\
	".\CNCControl.h"\
	".\ItemEdit.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	

"$(INTDIR)\ItemEdit.obj"	"$(INTDIR)\ItemEdit.sbr" : $(SOURCE) $(DEP_CPP_ITEME)\
 "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\MachinePathSegDlg.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_MACHI=\
	".\CNCControl.h"\
	".\MachinePathSegDlg.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\MachinePathSegDlg.obj" : $(SOURCE) $(DEP_CPP_MACHI) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_MACHI=\
	".\CNCControl.h"\
	".\MachinePathSegDlg.h"\
	".\Settings.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\MachinePathSegDlg.obj"	"$(INTDIR)\MachinePathSegDlg.sbr" : $(SOURCE)\
 $(DEP_CPP_MACHI) "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\MachinePropDlg.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_MACHIN=\
	".\CNCControl.h"\
	".\ControllerTracker.h"\
	".\MachinePropDlg.h"\
	".\PathDataObjects.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\MachinePropDlg.obj" : $(SOURCE) $(DEP_CPP_MACHIN) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_MACHIN=\
	".\CNCControl.h"\
	".\ControllerTracker.h"\
	".\MachinePropDlg.h"\
	".\PathDataObjects.h"\
	".\Settings.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\MachinePropDlg.obj"	"$(INTDIR)\MachinePropDlg.sbr" : $(SOURCE)\
 $(DEP_CPP_MACHIN) "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\MachineSim.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_MACHINE=\
	".\CNCControl.h"\
	".\ControllerTracker.h"\
	".\MachineSim.h"\
	".\Packet.h"\
	".\Param.h"\
	".\PathDataObjects.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\MachineSim.obj" : $(SOURCE) $(DEP_CPP_MACHINE) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_MACHINE=\
	".\CNCControl.h"\
	".\ControllerTracker.h"\
	".\MachineSim.h"\
	".\Packet.h"\
	".\Param.h"\
	".\PathDataObjects.h"\
	".\Settings.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\MachineSim.obj"	"$(INTDIR)\MachineSim.sbr" : $(SOURCE)\
 $(DEP_CPP_MACHINE) "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\MainFrm.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_MAINF=\
	"..\graph\plot.h"\
	".\CNCComms.h"\
	".\CNCControl.h"\
	".\CNCControlApp.h"\
	".\CommsDataDlg.h"\
	".\ContPathData.h"\
	".\Controller.h"\
	".\ControllerPath.h"\
	".\ControllerTracker.h"\
	".\ControlStatusDlg.h"\
	".\DiagnoseDlg.h"\
	".\MachinePathSegDlg.h"\
	".\MachinePropDlg.h"\
	".\MachineSim.h"\
	".\MainFrm.h"\
	".\Packet.h"\
	".\Param.h"\
	".\PathDataObjects.h"\
	".\PathDoc.h"\
	".\PathMove.h"\
	".\PathSpeed.h"\
	".\PathTimeStep.h"\
	".\PathTracker.h"\
	".\PktControl.h"\
	".\PlayerBar.h"\
	".\SeekValue.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"colors.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"PointSumInfo.h"\
	{$(INCLUDE)}"PolySegFit.h"\
	{$(INCLUDE)}"PolySegInfo.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\MainFrm.obj" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_MAINF=\
	"..\graph\plot.h"\
	".\CNCComms.h"\
	".\CNCControl.h"\
	".\CNCControlApp.h"\
	".\CommsDataDlg.h"\
	".\ContPathData.h"\
	".\Controller.h"\
	".\ControllerPath.h"\
	".\ControllerTracker.h"\
	".\ControlStatusDlg.h"\
	".\DiagnoseDlg.h"\
	".\MachinePathSegDlg.h"\
	".\MachinePropDlg.h"\
	".\MachineSim.h"\
	".\MainFrm.h"\
	".\Packet.h"\
	".\Param.h"\
	".\PathDataObjects.h"\
	".\PathDoc.h"\
	".\PathMove.h"\
	".\PathSpeed.h"\
	".\PathTimeStep.h"\
	".\PathTracker.h"\
	".\PktControl.h"\
	".\PlayerBar.h"\
	".\SeekValue.h"\
	".\Settings.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"colors.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"PointSumInfo.h"\
	{$(INCLUDE)}"PolySegFit.h"\
	{$(INCLUDE)}"PolySegInfo.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\MainFrm.obj"	"$(INTDIR)\MainFrm.sbr" : $(SOURCE) $(DEP_CPP_MAINF)\
 "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=..\Common\MoveView.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_MOVEV=\
	{$(INCLUDE)}"colors.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"MoveView.h"\
	{$(INCLUDE)}"StdAfx.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\MoveView.obj" : $(SOURCE) $(DEP_CPP_MOVEV) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_MOVEV=\
	{$(INCLUDE)}"colors.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"MoveView.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\MoveView.obj"	"$(INTDIR)\MoveView.sbr" : $(SOURCE) $(DEP_CPP_MOVEV)\
 "$(INTDIR)" "$(INTDIR)\CNCControl.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Packet.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_PACKE=\
	".\CNCControl.h"\
	".\ControllerTracker.h"\
	".\Packet.h"\
	".\Param.h"\
	".\PathDataObjects.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\Packet.obj" : $(SOURCE) $(DEP_CPP_PACKE) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_PACKE=\
	".\CNCControl.h"\
	".\ControllerTracker.h"\
	".\Packet.h"\
	".\Param.h"\
	".\PathDataObjects.h"\
	".\Settings.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\Packet.obj"	"$(INTDIR)\Packet.sbr" : $(SOURCE) $(DEP_CPP_PACKE)\
 "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\Param.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_PARAM=\
	".\CNCControl.h"\
	".\Param.h"\
	".\StdAfx.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	

"$(INTDIR)\Param.obj" : $(SOURCE) $(DEP_CPP_PARAM) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_PARAM=\
	".\CNCControl.h"\
	".\Param.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	

"$(INTDIR)\Param.obj"	"$(INTDIR)\Param.sbr" : $(SOURCE) $(DEP_CPP_PARAM)\
 "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\ParamDoc.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_PARAMD=\
	".\CNCComms.h"\
	".\CNCControl.h"\
	".\CommsDataDlg.h"\
	".\ContPathData.h"\
	".\Controller.h"\
	".\ControllerTracker.h"\
	".\MachineSim.h"\
	".\Packet.h"\
	".\Param.h"\
	".\ParamDoc.h"\
	".\PathDataObjects.h"\
	".\PktControl.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\ParamDoc.obj" : $(SOURCE) $(DEP_CPP_PARAMD) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_PARAMD=\
	".\CNCComms.h"\
	".\CNCControl.h"\
	".\CommsDataDlg.h"\
	".\ContPathData.h"\
	".\Controller.h"\
	".\ControllerTracker.h"\
	".\MachineSim.h"\
	".\Packet.h"\
	".\Param.h"\
	".\ParamDoc.h"\
	".\PathDataObjects.h"\
	".\PktControl.h"\
	".\Settings.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\ParamDoc.obj"	"$(INTDIR)\ParamDoc.sbr" : $(SOURCE) $(DEP_CPP_PARAMD)\
 "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\ParamLoader.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_PARAML=\
	".\CNCControl.h"\
	".\ContPathData.h"\
	".\ControllerTracker.h"\
	".\Packet.h"\
	".\Param.h"\
	".\ParamDoc.h"\
	".\ParamLoader.h"\
	".\PathDataObjects.h"\
	".\PktControl.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\StrUtils.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\ParamLoader.obj" : $(SOURCE) $(DEP_CPP_PARAML) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_PARAML=\
	".\CNCControl.h"\
	".\ContPathData.h"\
	".\ControllerTracker.h"\
	".\Packet.h"\
	".\Param.h"\
	".\ParamDoc.h"\
	".\ParamLoader.h"\
	".\PathDataObjects.h"\
	".\PktControl.h"\
	".\Settings.h"\
	".\StrUtils.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\ParamLoader.obj"	"$(INTDIR)\ParamLoader.sbr" : $(SOURCE)\
 $(DEP_CPP_PARAML) "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\ParamView.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_PARAMV=\
	".\CNCComms.h"\
	".\CNCControl.h"\
	".\CommsDataDlg.h"\
	".\ContPathData.h"\
	".\Controller.h"\
	".\ControllerTracker.h"\
	".\GridView.h"\
	".\MachineSim.h"\
	".\Packet.h"\
	".\Param.h"\
	".\ParamDoc.h"\
	".\ParamView.h"\
	".\PathDataObjects.h"\
	".\PktControl.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\ParamView.obj" : $(SOURCE) $(DEP_CPP_PARAMV) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_PARAMV=\
	".\CNCComms.h"\
	".\CNCControl.h"\
	".\CommsDataDlg.h"\
	".\ContPathData.h"\
	".\Controller.h"\
	".\ControllerTracker.h"\
	".\GridView.h"\
	".\MachineSim.h"\
	".\Packet.h"\
	".\Param.h"\
	".\ParamDoc.h"\
	".\ParamView.h"\
	".\PathDataObjects.h"\
	".\PktControl.h"\
	".\Settings.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\ParamView.obj"	"$(INTDIR)\ParamView.sbr" : $(SOURCE)\
 $(DEP_CPP_PARAMV) "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\PathAnimateView.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_PATHA=\
	"..\graph\plot.h"\
	".\AnimationSettingsDlg.h"\
	".\CNCControl.h"\
	".\ControllerTracker.h"\
	".\MainFrm.h"\
	".\PathAnimateView.h"\
	".\PathDataObjects.h"\
	".\PathDoc.h"\
	".\PathMove.h"\
	".\PathPositionDlg.h"\
	".\PathSpeedDoc.h"\
	".\PathTracker.h"\
	".\PathView.h"\
	".\PlayerBar.h"\
	".\SeekValue.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"colors.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"MoveView.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\PathAnimateView.obj" : $(SOURCE) $(DEP_CPP_PATHA) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_PATHA=\
	"..\graph\plot.h"\
	".\AnimationSettingsDlg.h"\
	".\CNCControl.h"\
	".\ControllerTracker.h"\
	".\MainFrm.h"\
	".\PathAnimateView.h"\
	".\PathDataObjects.h"\
	".\PathDoc.h"\
	".\PathMove.h"\
	".\PathPositionDlg.h"\
	".\PathSpeedDoc.h"\
	".\PathTracker.h"\
	".\PathView.h"\
	".\PlayerBar.h"\
	".\SeekValue.h"\
	".\Settings.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"colors.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"MoveView.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\PathAnimateView.obj"	"$(INTDIR)\PathAnimateView.sbr" : $(SOURCE)\
 $(DEP_CPP_PATHA) "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\PathDataObjects.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_PATHD=\
	".\CNCControl.h"\
	".\PathDataObjects.h"\
	".\StdAfx.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\PathDataObjects.obj" : $(SOURCE) $(DEP_CPP_PATHD) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_PATHD=\
	".\CNCControl.h"\
	".\PathDataObjects.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\PathDataObjects.obj"	"$(INTDIR)\PathDataObjects.sbr" : $(SOURCE)\
 $(DEP_CPP_PATHD) "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\PathDoc.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_PATHDO=\
	".\CNCComms.h"\
	".\CNCControl.h"\
	".\CNCControlApp.h"\
	".\CommsDataDlg.h"\
	".\ContPathData.h"\
	".\Controller.h"\
	".\ControllerPath.h"\
	".\ControllerTracker.h"\
	".\ControlStatusDlg.h"\
	".\MachineSim.h"\
	".\Packet.h"\
	".\Param.h"\
	".\PathDataObjects.h"\
	".\PathDoc.h"\
	".\PathLoader.h"\
	".\PathMove.h"\
	".\PathSpeed.h"\
	".\PathSpeedDoc.h"\
	".\PathTimeStep.h"\
	".\PathTracker.h"\
	".\PktControl.h"\
	".\SeekValue.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\StrUtils.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"PointSumInfo.h"\
	{$(INCLUDE)}"PolySegFit.h"\
	{$(INCLUDE)}"PolySegInfo.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\PathDoc.obj" : $(SOURCE) $(DEP_CPP_PATHDO) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_PATHDO=\
	".\CNCComms.h"\
	".\CNCControl.h"\
	".\CNCControlApp.h"\
	".\CommsDataDlg.h"\
	".\ContPathData.h"\
	".\Controller.h"\
	".\ControllerPath.h"\
	".\ControllerTracker.h"\
	".\ControlStatusDlg.h"\
	".\MachineSim.h"\
	".\Packet.h"\
	".\Param.h"\
	".\PathDataObjects.h"\
	".\PathDoc.h"\
	".\PathLoader.h"\
	".\PathMove.h"\
	".\PathSpeed.h"\
	".\PathSpeedDoc.h"\
	".\PathTimeStep.h"\
	".\PathTracker.h"\
	".\PktControl.h"\
	".\SeekValue.h"\
	".\Settings.h"\
	".\StrUtils.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"PointSumInfo.h"\
	{$(INCLUDE)}"PolySegFit.h"\
	{$(INCLUDE)}"PolySegInfo.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\PathDoc.obj"	"$(INTDIR)\PathDoc.sbr" : $(SOURCE) $(DEP_CPP_PATHDO)\
 "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\PathLoader.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_PATHL=\
	".\CNCControl.h"\
	".\PathDataObjects.h"\
	".\PathDoc.h"\
	".\PathLoader.h"\
	".\StdAfx.h"\
	".\StrUtils.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\PathLoader.obj" : $(SOURCE) $(DEP_CPP_PATHL) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_PATHL=\
	".\CNCControl.h"\
	".\PathDataObjects.h"\
	".\PathDoc.h"\
	".\PathLoader.h"\
	".\StrUtils.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\PathLoader.obj"	"$(INTDIR)\PathLoader.sbr" : $(SOURCE)\
 $(DEP_CPP_PATHL) "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\PathMove.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_PATHM=\
	".\CNCControl.h"\
	".\PathDataObjects.h"\
	".\PathDoc.h"\
	".\PathMove.h"\
	".\PathTracker.h"\
	".\SeekValue.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\PathMove.obj" : $(SOURCE) $(DEP_CPP_PATHM) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_PATHM=\
	".\CNCControl.h"\
	".\PathDataObjects.h"\
	".\PathDoc.h"\
	".\PathMove.h"\
	".\PathTracker.h"\
	".\SeekValue.h"\
	".\Settings.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\PathMove.obj"	"$(INTDIR)\PathMove.sbr" : $(SOURCE) $(DEP_CPP_PATHM)\
 "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\PathNCIDoc.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_PATHN=\
	".\CNCControl.h"\
	".\PathDataObjects.h"\
	".\PathDoc.h"\
	".\PathNCIDoc.h"\
	".\ReadNCIFile.h"\
	".\StdAfx.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"PointSumInfo.h"\
	{$(INCLUDE)}"PolyFunc.h"\
	{$(INCLUDE)}"PolySegParaFit.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\PathNCIDoc.obj" : $(SOURCE) $(DEP_CPP_PATHN) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_PATHN=\
	".\CNCControl.h"\
	".\PathDataObjects.h"\
	".\PathDoc.h"\
	".\PathNCIDoc.h"\
	".\ReadNCIFile.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"PointSumInfo.h"\
	{$(INCLUDE)}"PolyFunc.h"\
	{$(INCLUDE)}"PolySegParaFit.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\PathNCIDoc.obj"	"$(INTDIR)\PathNCIDoc.sbr" : $(SOURCE)\
 $(DEP_CPP_PATHN) "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\PathPositionDlg.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_PATHP=\
	".\CNCControl.h"\
	".\PathPositionDlg.h"\
	".\StdAfx.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	

"$(INTDIR)\PathPositionDlg.obj" : $(SOURCE) $(DEP_CPP_PATHP) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_PATHP=\
	".\CNCControl.h"\
	".\PathPositionDlg.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	

"$(INTDIR)\PathPositionDlg.obj"	"$(INTDIR)\PathPositionDlg.sbr" : $(SOURCE)\
 $(DEP_CPP_PATHP) "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\PathSpeed.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_PATHS=\
	".\PathDataObjects.h"\
	".\PathDoc.h"\
	".\PathSpeed.h"\
	".\PathTracker.h"\
	".\SeekValue.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\PathSpeed.obj" : $(SOURCE) $(DEP_CPP_PATHS) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_PATHS=\
	".\PathDataObjects.h"\
	".\PathDoc.h"\
	".\PathSpeed.h"\
	".\PathTracker.h"\
	".\SeekValue.h"\
	".\Settings.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\PathSpeed.obj"	"$(INTDIR)\PathSpeed.sbr" : $(SOURCE)\
 $(DEP_CPP_PATHS) "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\PathSpeedDoc.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_PATHSP=\
	".\CNCComms.h"\
	".\CNCControl.h"\
	".\CNCControlApp.h"\
	".\CommsDataDlg.h"\
	".\ContPathData.h"\
	".\Controller.h"\
	".\ControllerPath.h"\
	".\ControllerTracker.h"\
	".\ControlStatusDlg.h"\
	".\MachineSim.h"\
	".\Packet.h"\
	".\Param.h"\
	".\PathAnimateView.h"\
	".\PathDataObjects.h"\
	".\PathDoc.h"\
	".\PathMove.h"\
	".\PathPositionDlg.h"\
	".\PathSpeed.h"\
	".\PathSpeedDoc.h"\
	".\PathTimeStep.h"\
	".\PathTracker.h"\
	".\PathView.h"\
	".\PktControl.h"\
	".\SeekValue.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"MoveView.h"\
	{$(INCLUDE)}"PointSumInfo.h"\
	{$(INCLUDE)}"PolySegFit.h"\
	{$(INCLUDE)}"PolySegInfo.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\PathSpeedDoc.obj" : $(SOURCE) $(DEP_CPP_PATHSP) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_PATHSP=\
	".\CNCComms.h"\
	".\CNCControl.h"\
	".\CNCControlApp.h"\
	".\CommsDataDlg.h"\
	".\ContPathData.h"\
	".\Controller.h"\
	".\ControllerPath.h"\
	".\ControllerTracker.h"\
	".\ControlStatusDlg.h"\
	".\MachineSim.h"\
	".\Packet.h"\
	".\Param.h"\
	".\PathAnimateView.h"\
	".\PathDataObjects.h"\
	".\PathDoc.h"\
	".\PathMove.h"\
	".\PathPositionDlg.h"\
	".\PathSpeed.h"\
	".\PathSpeedDoc.h"\
	".\PathTimeStep.h"\
	".\PathTracker.h"\
	".\PathView.h"\
	".\PktControl.h"\
	".\SeekValue.h"\
	".\Settings.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"MoveView.h"\
	{$(INCLUDE)}"PointSumInfo.h"\
	{$(INCLUDE)}"PolySegFit.h"\
	{$(INCLUDE)}"PolySegInfo.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\PathSpeedDoc.obj"	"$(INTDIR)\PathSpeedDoc.sbr" : $(SOURCE)\
 $(DEP_CPP_PATHSP) "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\PathSpeedView.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_PATHSPE=\
	".\CNCControl.h"\
	".\ControllerTracker.h"\
	".\PathDataObjects.h"\
	".\PathDoc.h"\
	".\PathMove.h"\
	".\PathSpeedDoc.h"\
	".\PathSpeedView.h"\
	".\PathTracker.h"\
	".\SeekValue.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"colors.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"MoveView.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\PathSpeedView.obj" : $(SOURCE) $(DEP_CPP_PATHSPE) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_PATHSPE=\
	".\CNCControl.h"\
	".\ControllerTracker.h"\
	".\PathDataObjects.h"\
	".\PathDoc.h"\
	".\PathMove.h"\
	".\PathSpeedDoc.h"\
	".\PathSpeedView.h"\
	".\PathTracker.h"\
	".\SeekValue.h"\
	".\Settings.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"colors.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"MoveView.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\PathSpeedView.obj"	"$(INTDIR)\PathSpeedView.sbr" : $(SOURCE)\
 $(DEP_CPP_PATHSPE) "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\PathTimeStep.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_PATHT=\
	".\ControllerTracker.h"\
	".\PathDataObjects.h"\
	".\PathDoc.h"\
	".\PathMove.h"\
	".\PathTimeStep.h"\
	".\PathTracker.h"\
	".\SeekValue.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"PointSumInfo.h"\
	{$(INCLUDE)}"PolySegFit.h"\
	{$(INCLUDE)}"PolySegInfo.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\PathTimeStep.obj" : $(SOURCE) $(DEP_CPP_PATHT) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_PATHT=\
	".\ControllerTracker.h"\
	".\PathDataObjects.h"\
	".\PathDoc.h"\
	".\PathMove.h"\
	".\PathTimeStep.h"\
	".\PathTracker.h"\
	".\SeekValue.h"\
	".\Settings.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"PointSumInfo.h"\
	{$(INCLUDE)}"PolySegFit.h"\
	{$(INCLUDE)}"PolySegInfo.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\PathTimeStep.obj"	"$(INTDIR)\PathTimeStep.sbr" : $(SOURCE)\
 $(DEP_CPP_PATHT) "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\PathTracker.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_PATHTR=\
	".\PathDataObjects.h"\
	".\PathDoc.h"\
	".\PathTracker.h"\
	".\SeekValue.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\PathTracker.obj" : $(SOURCE) $(DEP_CPP_PATHTR) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_PATHTR=\
	".\PathDataObjects.h"\
	".\PathDoc.h"\
	".\PathTracker.h"\
	".\SeekValue.h"\
	".\Settings.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\PathTracker.obj"	"$(INTDIR)\PathTracker.sbr" : $(SOURCE)\
 $(DEP_CPP_PATHTR) "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\PathView.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_PATHV=\
	".\CNCControl.h"\
	".\PathDataObjects.h"\
	".\PathDoc.h"\
	".\PathPositionDlg.h"\
	".\PathView.h"\
	".\StdAfx.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"colors.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"MoveView.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\PathView.obj" : $(SOURCE) $(DEP_CPP_PATHV) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_PATHV=\
	".\CNCControl.h"\
	".\PathDataObjects.h"\
	".\PathDoc.h"\
	".\PathPositionDlg.h"\
	".\PathView.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"colors.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"MoveView.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\PathView.obj"	"$(INTDIR)\PathView.sbr" : $(SOURCE) $(DEP_CPP_PATHV)\
 "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\PktControl.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_PKTCO=\
	".\CNCControl.h"\
	".\ContPathData.h"\
	".\ControllerTracker.h"\
	".\Packet.h"\
	".\Param.h"\
	".\PathDataObjects.h"\
	".\PktControl.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\PktControl.obj" : $(SOURCE) $(DEP_CPP_PKTCO) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_PKTCO=\
	".\CNCControl.h"\
	".\ContPathData.h"\
	".\ControllerTracker.h"\
	".\Packet.h"\
	".\Param.h"\
	".\PathDataObjects.h"\
	".\PktControl.h"\
	".\Settings.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"Buffer.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\PktControl.obj"	"$(INTDIR)\PktControl.sbr" : $(SOURCE)\
 $(DEP_CPP_PKTCO) "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\PlayerBar.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_PLAYE=\
	".\CNCControl.h"\
	".\PlayerBar.h"\
	".\StdAfx.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	

"$(INTDIR)\PlayerBar.obj" : $(SOURCE) $(DEP_CPP_PLAYE) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_PLAYE=\
	".\CNCControl.h"\
	".\PlayerBar.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	

"$(INTDIR)\PlayerBar.obj"	"$(INTDIR)\PlayerBar.sbr" : $(SOURCE)\
 $(DEP_CPP_PLAYE) "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\PortSelDlg.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_PORTS=\
	".\CNCControl.h"\
	".\PortSelDlg.h"\
	".\StdAfx.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	

"$(INTDIR)\PortSelDlg.obj" : $(SOURCE) $(DEP_CPP_PORTS) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_PORTS=\
	".\CNCControl.h"\
	".\PortSelDlg.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	

"$(INTDIR)\PortSelDlg.obj"	"$(INTDIR)\PortSelDlg.sbr" : $(SOURCE)\
 $(DEP_CPP_PORTS) "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\ReadNCIFile.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_READN=\
	".\CNCControl.h"\
	".\PathDataObjects.h"\
	".\PathDoc.h"\
	".\ReadNCIFile.h"\
	".\StdAfx.h"\
	".\StrUtils.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\ReadNCIFile.obj" : $(SOURCE) $(DEP_CPP_READN) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_READN=\
	".\CNCControl.h"\
	".\PathDataObjects.h"\
	".\PathDoc.h"\
	".\ReadNCIFile.h"\
	".\StrUtils.h"\
	".\UserResource.h"\
	{$(INCLUDE)}"commonresource.h"\
	{$(INCLUDE)}"Matrix.h"\
	{$(INCLUDE)}"Vector.h"\
	

"$(INTDIR)\ReadNCIFile.obj"	"$(INTDIR)\ReadNCIFile.sbr" : $(SOURCE)\
 $(DEP_CPP_READN) "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\SeekValue.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_SEEKV=\
	".\SeekValue.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\SeekValue.obj" : $(SOURCE) $(DEP_CPP_SEEKV) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_SEEKV=\
	".\SeekValue.h"\
	

"$(INTDIR)\SeekValue.obj"	"$(INTDIR)\SeekValue.sbr" : $(SOURCE)\
 $(DEP_CPP_SEEKV) "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

SOURCE=.\Settings.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_SETTI=\
	".\Settings.h"\
	".\StdAfx.h"\
	".\StrUtils.h"\
	{$(INCLUDE)}"Vector.h"\
	
CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D\
 "_AFXDLL" /Fp"$(INTDIR)\CNCControl.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\Settings.obj" : $(SOURCE) $(DEP_CPP_SETTI) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_SETTI=\
	".\Settings.h"\
	".\StrUtils.h"\
	{$(INCLUDE)}"Vector.h"\
	
CPP_SWITCHES=/nologo /MDd /W4 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D\
 "_WINDOWS" /D "_AFXDLL" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\CNCControl.pch"\
 /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\Settings.obj"	"$(INTDIR)\Settings.sbr" : $(SOURCE) $(DEP_CPP_SETTI)\
 "$(INTDIR)" "$(INTDIR)\CNCControl.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\StdAfx.cpp
DEP_CPP_STDAF=\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "CNCControl - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D\
 "_AFXDLL" /Fp"$(INTDIR)\CNCControl.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\CNCControl.pch" : $(SOURCE) $(DEP_CPP_STDAF)\
 "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W4 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D\
 "_WINDOWS" /D "_AFXDLL" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\CNCControl.pch"\
 /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr"	"$(INTDIR)\CNCControl.pch" : \
$(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\StrUtils.cpp

!IF  "$(CFG)" == "CNCControl - Win32 Release"

DEP_CPP_STRUT=\
	".\StdAfx.h"\
	".\StrUtils.h"\
	

"$(INTDIR)\StrUtils.obj" : $(SOURCE) $(DEP_CPP_STRUT) "$(INTDIR)"\
 "$(INTDIR)\CNCControl.pch"


!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

DEP_CPP_STRUT=\
	".\StrUtils.h"\
	

"$(INTDIR)\StrUtils.obj"	"$(INTDIR)\StrUtils.sbr" : $(SOURCE) $(DEP_CPP_STRUT)\
 "$(INTDIR)" "$(INTDIR)\CNCControl.pch"


!ENDIF 

!IF  "$(CFG)" == "CNCControl - Win32 Release"

"PathMath - Win32 Release" : 
   cd "\Derek\VC\PathMath"
   $(MAKE) /$(MAKEFLAGS) /F .\PathMath.mak CFG="PathMath - Win32 Release" 
   cd "..\CNCControl"

"PathMath - Win32 ReleaseCLEAN" : 
   cd "\Derek\VC\PathMath"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\PathMath.mak CFG="PathMath - Win32 Release"\
 RECURSE=1 
   cd "..\CNCControl"

!ELSEIF  "$(CFG)" == "CNCControl - Win32 Debug"

"PathMath - Win32 Debug" : 
   cd "\Derek\VC\PathMath"
   $(MAKE) /$(MAKEFLAGS) /F .\PathMath.mak CFG="PathMath - Win32 Debug" 
   cd "..\CNCControl"

"PathMath - Win32 DebugCLEAN" : 
   cd "\Derek\VC\PathMath"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\PathMath.mak CFG="PathMath - Win32 Debug"\
 RECURSE=1 
   cd "..\CNCControl"

!ENDIF 


!ENDIF 

