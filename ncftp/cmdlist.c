/* cmdlist.c
 *
 * Copyright (c) 1992-2002 by Mike Gleason.
 * All rights reserved.
 * 
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

#include "shell.h"
#include "bookmark.h"
#include "cmds.h"

/* These will be sorted lexiographically when the program is run, but
 * they should already be listed that way.
 */
Command gCommands[] = {
#if defined(WIN32) || defined(_WINDOWS)
#else
	{ "!",
		ShellCmd,
		"[arguments]",
		"Runs a subshell",
		kCmdHidden,
		kNoMin, kNoMax,
	},
#endif
	{ "?",
		HelpCmd,
		"[optional commands]",
		"shows commands, or detailed help on specified commands",
		kCmdHidden,
		kNoMin, kNoMax,
	},
	{ "ascii",
		TypeCmd,
		"",
		"sets the file transfer type to ASCII text",
		kCmdMustBeConnected,
		0, 0,
	},
	{ "bgget",
		SpoolGetCmd,
"[-flags] file1 [file2...]\n\
Flags:\n\
  -z   : Get the remote file X, and name it to Y.\n\
  -@ <time> : Wait until <time> to do the transfer.\n\
              It must be expressed as one of the following:\n\
	          YYYYMMDDHHMMSS\n\
	          \"now + N hours|min|sec|days\"\n\
	          HH:MM",
		"collects items to download later from the remote host",
		kCmdMustBeConnected | kCompleteRemoteFile,
		1, kNoMax,
	},
	{ "bgput",
		SpoolPutCmd,
"[-flags] file1 [file2...]\n\
Flags:\n\
  -z   : Send the local file X, and name the remote copy to Y.\n\
  -@ <time> : Wait until <time> to do the transfer.\n\
              It must be expressed as one of the following:\n\
	          YYYYMMDDHHMMSS\n\
	          \"now + N hours|min|sec|days\"\n\
	          HH:MM",
		"collects items to upload later to the remote host",
		kCmdMustBeConnected | kCompleteLocalFile,
		1, kNoMax,
	},
	{ "bgstart",
		BGStartCmd,
		"[n]",
		"starts a ncftpbatch process to process spooled files",
		0,
		0, 1,
	},
	{ "binary",
		TypeCmd,
		"",
		"sets the file transfer type to binary/image",
		kCmdMustBeConnected,
		0, 0,
	},
	{ "bookmark",
		BookmarkCmd,
		"[bookmark-name-to-save-as]",
		"Creates or updates a bookmark using the current host and directory",
		kCmdMustBeConnected | kCompleteBookmark,
		0, 1,
	},
	{ "bookmarks",
		(CmdProc) HostsCmd,
		"[-l]",
		"lets you edit the settings for each bookmark",
		0,
		kNoMin, kNoMax,
	},
	{ "bye",
		(CmdProc) QuitCmd,
		"",
		"exits NcFTP",
		kCmdHidden,
		0, 0,
	},
	{ "cat",
		CatCmd,
		"file1 [file2...]",
		"views a file from the remote host",
		kCmdMustBeConnected | kCompleteRemoteFile,
		1, kNoMax,
	},
	{ "cd",
		ChdirCmd,
		"<directory>",
		"changes remote working directory",
		kCmdMustBeConnected | kCompleteRemoteDir,
		0, 1,
	},
	{ "chmod",
		ChmodCmd,
		"mode file1 [file2...]",
		"changes permissions for files on the remote host",
		kCmdMustBeConnected | kCompleteRemoteFile,
		2, kNoMax,
	},
	{ "close",
		(CmdProc) CloseCmd,
		"",
		"closes the connection to the remote host",
		kCmdMustBeConnected,
		0, 0,
	},
	{ "debug",
		DebugCmd,
		"[debug level]",
		"sets debug mode to level x",
		0,
		kNoMin, kNoMax,
	},
	{ "delete",
		DeleteCmd,
		"file1 [file2...]",
		"deletes files from the remote host",
		kCmdMustBeConnected | kCmdHidden | kCompleteRemoteFile,
		1, kNoMax,
	},
	{ "dir",
		ListCmd,
		"[items to list]",
		"prints a verbose directory listing",
		kCmdMustBeConnected | kCompleteRemoteDir,
		kNoMin, kNoMax,
	},
	{ "echo",
		EchoCmd,
		"[items to echo]",
		"echos back to screen",
		kCmdHidden,
		kNoMin, kNoMax,
	},
	{ "exit",
		(CmdProc) QuitCmd,
		"",
		"quits NcFTP",
		kCmdHidden,
		0, 0,
	},
	{ "get",
		GetCmd,
"[-flags] file1 [file2...]\n\
Flags:\n\
  -R   : Recursive.  Useful for fetching whole directories.\n\
  -z   : Get the remote file X, and name it to Y.\n\
  -a   : Get files using ASCII mode.\n\
  -A   : Append entire remote file to the local file.\n\
  -f   : Force overwrite (do not try to auto-resume transfers).\n\
Examples:\n\
  get README\n\
  get README.*\n\
  get \"**Name with stars and spaces in it**\"\n\
  get -R new-files-directory\n\
  get -z WIN.INI ~/junk/windows-init-file",
		"fetches files from the remote host",
		kCmdMustBeConnected | kCompleteRemoteFile,
		1, kNoMax,
	},
	{ "help",
		HelpCmd,
		"[optional commands]",
		"shows commands, or detailed help on specified commands",
		0,
		kNoMin, kNoMax,
	},
	{ "hosts",
		(CmdProc) HostsCmd,
		"",
		"lets you edit the settings for each remote host",
		kCmdMustBeDisconnected | kCmdHidden,
		kNoMin, kNoMax,
	},
#if defined(WIN32) || defined(_WINDOWS)
#else
	{ "jobs",
		(CmdProc) JobsCmd,
		"",
		"shows status of background NcFTP tasks",
		0,
		0, 0,
	},
#endif
	{ "lcd",
		LocalChdirCmd,
		"<directory>",
		"changes local working directory",
		kCompleteLocalDir,
		kNoMin, 1,
	},
#if defined(WIN32) || defined(_WINDOWS)
#else
	{ "lchmod",
		LocalChmodCmd,
		"mode file1 [file2...]",
		"changes permissions for files on the local host",
		kCompleteLocalFile | kCompleteLocalDir,
		2, kNoMax,
	},
#endif
	{ "less",
		PageCmd,
		"file1 [file2...]",
		"views a file from the remote host one page at a time.",
		kCmdMustBeConnected | kCmdHidden | kCompleteRemoteFile,
		1, kNoMax,
	},
	{ "lls",
		LocalListCmd,
		"[items to list]",
		"prints a local directory listing",
		kCompleteLocalDir,
		kNoMin, kNoMax,
	},
	{ "lmkdir",
		(CmdProc) LocalMkdirCmd,
		"[directories]",
		"creates directories on the local host",
		0,
		1, kNoMax,
	},
	{ "lookup",
		LookupCmd,
		"[-v|-V] <host or IP number> [<more hosts or IP numbers>]",
		"looks up information in the host database",
		0,
		1, kNoMax,
	},
#if defined(WIN32) || defined(_WINDOWS)
#else
	{ "lpage",
		LocalPageCmd,
		"file1 [file2...]",
		"views a file on the local host one page at a time.",
		kCompleteLocalFile,
		1, kNoMax,
	},
#endif
	{ "lpwd",
		(CmdProc) LocalPwdCmd,
		"",
		"Prints the current local working directory",
		0,
		0, 0,
	},
	{ "lrename",
		LocalRenameCmd,
		"oldname newname",
		"changes the name of a file on the local host",
		kCompleteLocalFile | kCompleteLocalDir,
		2, 2,
	},
	{ "lrm",
		(CmdProc) LocalRmCmd,
		"[files]",
		"removes files on the local host",
		kCompleteLocalFile | kCompleteLocalDir,
		1, kNoMax,
	},
	{ "lrmdir",
		(CmdProc) LocalRmdirCmd,
		"[directories]",
		"removes directories on the local host",
		kCompleteLocalDir,
		1, kNoMax,
	},
	{ "ls",
		ListCmd,
		"[items to list]",
		"prints a remote directory listing",
		kCmdMustBeConnected | kCompleteRemoteDir,
		kNoMin, kNoMax,
	},
	{ "mget",
		GetCmd,
"[-flags] file1 [file2...]\n\
Flags:\n\
  -R   : Recursive.  Useful for fetching whole directories.\n\
  -z   : Get the remote file X, and name it to Y.\n\
  -a   : Get files using ASCII mode.\n\
  -A   : Append entire remote file to the local file.\n\
  -f   : Force overwrite (do not try to auto-resume transfers).\n\
Examples:\n\
  get README\n\
  get README.*\n\
  get \"**Name with stars and spaces in it**\"\n\
  get -R new-files-directory\n\
  get -z WIN.INI ~/junk/windows-init-file",
		"fetches files from the remote host",
		kCmdMustBeConnected | kCmdHidden | kCompleteRemoteFile,
		1, kNoMax,
	},
	{ "mkdir",
		MkdirCmd,
		"dir1 [dir2...]",
		"creates directories on the remote host",
		kCmdMustBeConnected,
		1, kNoMax,
	},
	{ "mls",
		MlsCmd,
		"[<directory to list> | -d <single item to list>]",
		"prints a machine-readable directory listing",
		kCmdMustBeConnected | kCompleteRemoteDir | kCmdHidden,
		kNoMin, kNoMax,
	},
	{ "more",
		PageCmd,
		"file1 [file2...]",
		"views a file from the remote host one page at a time.",
		kCmdMustBeConnected | kCmdHidden | kCompleteRemoteFile,
		1, kNoMax,
	},
	{ "mput",
		PutCmd,
"[-flags] file1 [file2...]\n\
Flags:\n\
  -z   : Send the local file X, and name the remote copy to Y.\n\
  -f   : Force overwrite (do not try to auto-resume transfers).\n\
  -a   : Send files using ASCII mode.\n\
  -A   : Append entire local file to the remote file.\n\
  -R   : Recursive.  Useful for sending whole directories.\n\
Examples:\n\
  put README\n\
  put -z ~/junk/windows-init-file WIN.INI",
		"sends files to the remote host",
		kCmdMustBeConnected | kCompleteLocalFile | kCmdHidden,
		1, kNoMax,
	},
	{ "page",
		PageCmd,
		"file1 [file2...]",
		"views a file from the remote host one page at a time.",
		kCmdMustBeConnected | kCompleteRemoteFile,
		1, kNoMax,
	},
	{ "open",
		OpenCmd,
"[-flags] [sitename]\n\
Flags:\n\
  -a    : Open anonymously.\n\
  -u XX : Login with username XX.\n\
  -p XX : Login with password XX.\n\
  -j XX : Login with account XX.\n\
  -P XX : Use port number X when opening.\n\
Examples:\n\
  open sphygmomanometer.unl.edu\n\
  open -u mario bowser.nintendo.co.jp\n",
		"connects to a remote host",
		kCompleteBookmark,
		kNoMin, kNoMax,
	},
	{ "pdir",
		ListCmd,
		"[items to list]",
		"views a directory listing through your pager",
		kCmdMustBeConnected | kCompleteRemoteDir,
		kNoMin, kNoMax,
	},
	{ "pls",
		ListCmd,
		"[items to list]",
		"views a directory listing through your pager",
		kCmdMustBeConnected | kCompleteRemoteDir,
		kNoMin, kNoMax,
	},
	{ "prefs",
		(CmdProc) SetCmd,
		"",
		"shows the program's settings",
		kCmdHidden,
		0, 0,
	},
	{ "put",
		PutCmd,
"[-flags] file1 [file2...]\n\
Flags:\n\
  -z   : Send the local file X, and name the remote copy to Y.\n\
  -f   : Force overwrite (do not try to auto-resume transfers).\n\
  -a   : Send files using ASCII mode.\n\
  -A   : Append entire local file to the remote file.\n\
  -R   : Recursive.  Useful for sending whole directories.\n\
Examples:\n\
  put README\n\
  put -z ~/junk/windows-init-file WIN.INI",
		"sends a file to the remote host",
		kCmdMustBeConnected | kCompleteLocalFile,
		1, kNoMax,
	},
	{ "pwd",
		(CmdProc) PwdCmd,
		"",
		"Prints the current remote working directory",
		kCmdMustBeConnected,
		0, 0,
	},
	{ "quit",
		(CmdProc) QuitCmd,
		"",
		"take a wild guess",
		0,
		0, 0,
	},
	{ "quote",
		QuoteCmd,
		"command-string",
		"sends an FTP command to the remote server",
		kCmdMustBeConnected,
		1, kNoMax,
	},
	{ "rename",
		RenameCmd,
		"oldname newname",
		"changes the name of a file on the remote host",
		kCmdMustBeConnected | kCompleteRemoteFile,
		2, 2,
	},
	{ "rglob",
		RGlobCmd,
		"regex",
		"tests remote filename wildcard matching",
		kCmdMustBeConnected | kCmdHidden,
		1, kNoMax,
	},
	{ "rhelp",
		RmtHelpCmd,
		"[help string]",
		"requests help from the remote server",
		kCmdMustBeConnected,
		kNoMin, kNoMax,
	},
	{ "rm",
		DeleteCmd,
		"[-r] file1 [file2...]",
		"deletes files from the remote host",
		kCmdMustBeConnected | kCompleteRemoteFile,
		1, kNoMax,
	},
	{ "rmdir",
		RmdirCmd,
		"dir1 [dir2...]",
		"deletes directories from the remote host",
		kCmdMustBeConnected | kCompleteRemoteDir,
		1, kNoMax,
	},
	{ "set",
		SetCmd,
		"[option [newvalue]]",
		"lets you configure a program setting from the command line",
		kCompletePrefOpt,
		0, 2,
	},
	{ "show",
		(CmdProc) SetCmd,
		"[option]",
		"shows one or more the program's settings",
		kCompletePrefOpt,
		0, 1,
	},
	{ "site",
		SiteCmd,
		"command-string",
		"sends a host-specific FTP command to the remote server",
		kCmdMustBeConnected,
		1, kNoMax,
	},
	{ "symlink",
		SymlinkCmd,
		"existing-item link-item",
		"creates a symbolic link on the remote host",
		kCmdHidden | kCmdMustBeConnected | kCompleteRemoteFile,
		2, 2,
	},
	{ "type",
		TypeCmd,
		"[ascii | binary | image]",
		"sets file transfer type (one of 'ascii' or 'binary')",
		kCmdMustBeConnected,
		0, 1,
	},
	{ "umask",
		UmaskCmd,
		"mask",
		"sets the process umask on remote host",
		kCmdMustBeConnected,
		1, 1,
	},
	{ "version",
		(CmdProc) VersionCmd,
		"",
		"prints version information",
		0,
		kNoMin, kNoMax,
	},
};

size_t gNumCommands = ((size_t) (sizeof(gCommands) / sizeof(Command)));

/* eof */
