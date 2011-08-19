#include <QtGui>
#include "GameList.h"
#include "MainWindow.h"
#include "LogWindow.h"
#include "Resources.h"
#include "ConfigMain.h"
#include "IssueReporter.h"

#include "BootManager.h"
#include "Common.h"
#include "ConfigManager.h"
#include "Core.h"


void DMainWindow::StartGame(const std::string& filename)
{
	// TODO: Disable play toolbar action, replace with pause

	renderWindow = new DRenderWindow;
	renderWindow->setWindowTitle(tr("Dolphin")); // TODO: Other window title..

	// TODO: When rendering to main, this won't resize the parent window..
	if (!SConfig::GetInstance().m_LocalCoreStartupParameter.bRenderToMain)
	{
		// TODO: render window should be dockable into the main window? => render to main window
		connect(renderWindow, SIGNAL(Closed()), this, SLOT(OnStop()));
		renderWindow->move(SConfig::GetInstance().m_LocalCoreStartupParameter.iRenderWindowXPos,
							SConfig::GetInstance().m_LocalCoreStartupParameter.iRenderWindowYPos);
		renderWindow->resize(SConfig::GetInstance().m_LocalCoreStartupParameter.iRenderWindowWidth, // TODO: Make sure these are client sizes!
							SConfig::GetInstance().m_LocalCoreStartupParameter.iRenderWindowHeight);
		renderWindow->show();
	}
	else
	{
		centralLayout->addWidget(renderWindow);
	}

	if (!BootManager::BootCore(filename))
	{
		QMessageBox(QMessageBox::Critical, tr("Fatal error"), tr("Failed to init Core"), QMessageBox::Ok, this);
		// Destroy the renderer frame when not rendering to main
		if (!SConfig::GetInstance().m_LocalCoreStartupParameter.bRenderToMain)
			renderWindow->close(); // TODO: .. meh, this will conflict with the code above, endless loop and stuff
		renderWindow = NULL;
	}
	else
	{
		// TODO: Disable screensaver!

		// TODO: Fullscreen
		//DoFullscreen(SConfig::GetInstance().m_LocalCoreStartupParameter.bFullscreen);

		renderWindow->setFocus();
		emit CoreStateChanged(Core::CORE_RUN);
	}
}

std::string DMainWindow::RequestBootFilename()
{
	// If a game is already selected, just return the filename
	if (gameList->GetSelectedISO() != NULL)
		return gameList->GetSelectedISO()->GetFileName();

	// Otherwise, try the default ISO and then the previously booted ISO
	SCoreStartupParameter& StartUp = SConfig::GetInstance().m_LocalCoreStartupParameter;
	std::string filename;

	if (!StartUp.m_strDefaultGCM.empty() && File::Exists(StartUp.m_strDefaultGCM.c_str()))
		return StartUp.m_strDefaultGCM;

	if (!SConfig::GetInstance().m_LastFilename.empty() && File::Exists(SConfig::GetInstance().m_LastFilename.c_str()))
		return SConfig::GetInstance().m_LastFilename;

	// TODO: m_GameListCtrl->BrowseForDirectory() and return the selected ISO

	return std::string();
}

void DMainWindow::DoStartPause()
{
    if (Core::GetState() == Core::CORE_RUN)
    {
        Core::SetState(Core::CORE_PAUSE);
		emit CoreStateChanged(Core::CORE_PAUSE);

		// TODO: This doesn't work, yet
		if (SConfig::GetInstance().m_LocalCoreStartupParameter.bHideCursor)
			renderWindow->setCursor(Qt::BlankCursor);
    }
    else
    {
        Core::SetState(Core::CORE_RUN);
		emit CoreStateChanged(Core::CORE_RUN);

		// TODO: This doesn't work, yet
		if (SConfig::GetInstance().m_LocalCoreStartupParameter.bHideCursor)
			renderWindow->setCursor(Qt::BlankCursor);
    }
}

void DMainWindow::DoStop()
{
	if (Core::GetState() != Core::CORE_UNINITIALIZED && !is_stopping)
	{
		is_stopping = true;
		// Ask for confirmation in case the user accidentally clicked Stop / Escape
		if (SConfig::GetInstance().m_LocalCoreStartupParameter.bConfirmStop)
		{
			int ret = QMessageBox::question(this, tr("Please confirm..."), tr("Do you want to stop the current emulation?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

			if (ret == QMessageBox::No)
				return;
		}

		// TODO: Movie stuff
		// TODO: Show the author/description dialog here
		/*if(Movie::IsRecordingInput())
			DoRecordingSave();
		if(Movie::IsPlayingInput() || Movie::IsRecordingInput())
			Movie::EndPlayInput(false);*/

		// TODO: Show busy cursor
		BootManager::Stop();
		// TODO: Hide busy cursor again

		// TODO: Allow screensaver again
		// TODO: Restore original window title

		renderWindow->close();
		renderWindow = NULL;

		// TODO: Show cursor again if it was hidden before (SConfig::GetInstance().m_LocalCoreStartupParameter.bHideCursor)

		// TODO: Return from fullscreen if necessary (DoFullscreen in the wx code)

		// TODO:
		// If batch mode was specified on the command-line, exit now.
		//if (m_bBatchMode)
		//	Close(true);

		// TODO:
        // If using auto size with render to main, reset the application size.
/*        if (SConfig::GetInstance().m_LocalCoreStartupParameter.bRenderToMain &&
                SConfig::GetInstance().m_LocalCoreStartupParameter.bRenderWindowAutoSize)
            SetSize(SConfig::GetInstance().m_LocalCoreStartupParameter.iWidth,
                    SConfig::GetInstance().m_LocalCoreStartupParameter.iHeight);*/

		emit CoreStateChanged(Core::CORE_UNINITIALIZED);
	}
	is_stopping = false;
}

void DMainWindow::OnStartPause()
{
	if (Core::GetState() != Core::CORE_UNINITIALIZED)
    {
		// TODO:
		/*
		if (UseDebugger)
        {
            if (CCPU::IsStepping())
                CCPU::EnableStepping(false);
            else
                CCPU::EnableStepping(true);  // Break

            wxThread::Sleep(20);
            g_pCodeWindow->JumpToAddress(PC);
            g_pCodeWindow->Update();
            // Update toolbar with Play/Pause status
            UpdateGUI();
        }
        else*/
			DoStartPause();
    }
    else
	{
		// initialize Core and boot the game
		std::string filename = RequestBootFilename();
		if (filename.empty())
			QMessageBox::critical(this, tr("Error"), tr("No boot image selected"));
		else
			StartGame(filename);
	}
}

void DMainWindow::OnStop()
{
	DoStop();
}

void DMainWindow::OnCoreStateChanged(Core::EState state)
{
	bool is_not_initialized = state == Core::CORE_UNINITIALIZED;
	bool is_running = state == Core::CORE_RUN;
	bool is_paused = state == Core::CORE_PAUSE;

	// Tool bar
	playAction->setEnabled(is_not_initialized || is_running || is_paused);
	if (is_running)
		playAction->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
	else if (is_paused || is_not_initialized)
		playAction->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));

	stopAction->setEnabled(is_running || is_paused);

	// Game list
	gameList->setEnabled(is_not_initialized);
	gameList->setVisible(is_not_initialized);

	// TODO: Update menu items
}

void DMainWindow::OnLoadIso()
{
	QString selection = QFileDialog::getOpenFileName(this, tr("Select an ISO"));
	if(selection.length()) QMessageBox::information(this, tr("Notice"), selection, QMessageBox::Ok);
	// TODO?
}

void DMainWindow::OnRefreshList()
{
	// TODO: Make the progress bar stay enabled
	setEnabled(false);
	gameList->ScanForIsos();
	setEnabled(true);
}

void DMainWindow::OnShowLogMan(bool show)
{
	if (show)
	{
		logWindow->setVisible(true);
		SConfig::GetInstance().m_InterfaceLogWindow = true;
	}
	else if (logWindow)
	{
		logWindow->setVisible(false);
		SConfig::GetInstance().m_InterfaceLogWindow = false;
	}
}

void DMainWindow::OnShowLogSettings(bool show)
{
	if (show)
	{
		logSettings->setVisible(true);
		SConfig::GetInstance().m_InterfaceLogConfigWindow = true;
	}
	else if (logSettings)
	{
		logSettings->setVisible(false);
		SConfig::GetInstance().m_InterfaceLogConfigWindow= false;
	}
}

void DMainWindow::OnHideMenu(bool hide)
{
	menuBar()->setVisible(!hide);
}

void DMainWindow::OnAbout()
{
	QMessageBox::about(this, tr("About Dolphin"),
						tr(	"Dolphin SVN revision %1\n"
							"Copyright (c) 2003-2010+ Dolphin Team\n"
							"\n"
							"Dolphin is a Gamecube/Wii emulator, which was\n"
							"originally written by F|RES and ector.\n"
							"Today Dolphin is an open source project with many\n"
							"contributors, too many to list.\n"
							"If interested, just go check out the project page at\n"
							"http://code.google.com/p/dolphin-emu/ .\n"
							"\n"
							"Special thanks to Bushing, Costis, CrowTRobo,\n"
							"Marcan, Segher, Titanik, or9 and Hotquik for their\n"
							"reverse engineering and docs/demos.\n"
							"\n"
							"Big thanks to Gilles Mouchard whose Microlib PPC\n"
							"emulator gave our development a kickstart.\n"
							"\n"
							"Thanks to Frank Wille for his PowerPC disassembler,\n"
							"which or9 and we modified to include Gekko specifics.\n"
							"\n"
							"Thanks to hcs/destop for their GC ADPCM decoder.\n"
							"\n"
							"We are not affiliated with Nintendo in any way.\n"
							"Gamecube and Wii are trademarks of Nintendo.\n"
							"The emulator is for educational purposes only\n"
							"and should not be used to play games you do\n"
							"not legally own.").arg(svn_rev_str));
}

void DMainWindow::OnLogWindowClosed()
{
	// this calls OnShowLogMan(false)
	showLogManAct->setChecked(false);
}

void DMainWindow::OnLogSettingsWindowClosed()
{
	// this calls OnShowLogMan(false)
	showLogSettingsAct->setChecked(false);
}

void DMainWindow::OnConfigure()
{
	DConfigDialog* dialog = new DConfigDialog(DConfigDialog::ICI_General, this);
	dialog->show();
}

void DMainWindow::OnGfxSettings()
{
	DConfigDialog* dialog = new DConfigDialog(DConfigDialog::ICI_Graphics, this);
	dialog->show();
}

void DMainWindow::OnSoundSettings()
{
	DConfigDialog* dialog = new DConfigDialog(DConfigDialog::ICI_Sound, this);
	dialog->show();
}

void DMainWindow::OnGCPadSettings()
{
	DConfigDialog* dialog = new DConfigDialog(DConfigDialog::ICI_GCPad, this);
	dialog->show();
}

void DMainWindow::OnWiimoteSettings()
{
	DConfigDialog* dialog = new DConfigDialog(DConfigDialog::ICI_Wiimote, this);
	dialog->show();
}

void DMainWindow::OnReportIssue()
{
	DIssueReporter* reporter = new DIssueReporter(this);
	reporter->show();
}
