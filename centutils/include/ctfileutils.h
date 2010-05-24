/* Editor Settings: expandtabs and use 4 spaces for indentation
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 * -*- mode: c, c-basic-offset: 4 -*- */

/*
 * Copyright Likewise Software    2004-2008
 * All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the license, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
 * General Public License for more details.  You should have received a copy
 * of the GNU Lesser General Public License along with this program.  If
 * not, see <http://www.gnu.org/licenses/>.
 *
 * LIKEWISE SOFTWARE MAKES THIS SOFTWARE AVAILABLE UNDER OTHER LICENSING
 * TERMS AS WELL.  IF YOU HAVE ENTERED INTO A SEPARATE LICENSE AGREEMENT
 * WITH LIKEWISE SOFTWARE, THEN YOU MAY ELECT TO USE THE SOFTWARE UNDER THE
 * TERMS OF THAT SOFTWARE LICENSE AGREEMENT INSTEAD OF THE TERMS OF THE GNU
 * LESSER GENERAL PUBLIC LICENSE, NOTWITHSTANDING THE ABOVE NOTICE.  IF YOU
 * HAVE QUESTIONS, OR WISH TO REQUEST A COPY OF THE ALTERNATE LICENSING
 * TERMS OFFERED BY LIKEWISE SOFTWARE, PLEASE CONTACT LIKEWISE SOFTWARE AT
 * license@likewisesoftware.com
 */

/*
 * Copyright Likewise Software    2004-2008
 * All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the license, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
 * General Public License for more details.  You should have received a copy
 * of the GNU Lesser General Public License along with this program.  If
 * not, see <http://www.gnu.org/licenses/>.
 *
 * LIKEWISE SOFTWARE MAKES THIS SOFTWARE AVAILABLE UNDER OTHER LICENSING
 * TERMS AS WELL.  IF YOU HAVE ENTERED INTO A SEPARATE LICENSE AGREEMENT
 * WITH LIKEWISE SOFTWARE, THEN YOU MAY ELECT TO USE THE SOFTWARE UNDER THE
 * TERMS OF THAT SOFTWARE LICENSE AGREEMENT INSTEAD OF THE TERMS OF THE GNU
 * LESSER GENERAL PUBLIC LICENSE, NOTWITHSTANDING THE ABOVE NOTICE.  IF YOU
 * HAVE QUESTIONS, OR WISH TO REQUEST A COPY OF THE ALTERNATE LICENSING
 * TERMS OFFERED BY LIKEWISE SOFTWARE, PLEASE CONTACT LIKEWISE SOFTWARE AT
 * license@likewisesoftware.com
 */

#ifndef __CTFILEUTILS_H__
#define __CTFILEUTILS_H__


LW_BEGIN_EXTERN_C

CENTERROR
CTRemoveFile(
    PCSTR pszPath
    );

CENTERROR
CTRemoveDirectory(
    PCSTR pszPath
    );

CENTERROR
CTCheckFileExists(
    PCSTR pszPath,
    PBOOLEAN pbFileExists
    );

CENTERROR
CTCheckLinkExists(
    PCSTR pszPath,
    PBOOLEAN pbLinkExists
    );

CENTERROR
CTCheckFileOrLinkExists(
    PCSTR pszPath,
    PBOOLEAN pbExists
    );

CENTERROR
CTGetSymLinkTarget(
   PCSTR pszPath,
   PSTR* ppszTargetPath
   );

CENTERROR
CTCheckDirectoryExists(
    PCSTR pszPath,
    PBOOLEAN pbFileExists
    );

CENTERROR
CTCheckSockExists(
    PCSTR pszPath,
    PBOOLEAN pbSockExists
    );

CENTERROR
CTCreateSymLink(
    PCSTR pszOldPath,
    PCSTR pszNewPath
    );

CENTERROR
CTMoveFile(
    PCSTR pszSrcPath,
    PCSTR pszDstPath
    );

CENTERROR
CTMoveFileEx(
    PCSTR pszSrcPath,
    PCSTR pszDstPath,
    BOOLEAN bCrossDevice
    );

CENTERROR
CTSafeReplaceFile(
        PCSTR finalName,
        PCSTR replaceWith);

CENTERROR
CTCopyFileWithPerms(
    PCSTR pszSrcPath,
    PCSTR pszDstPath,
    mode_t dwPerms
    );
    
CENTERROR
CTGetOwnerUID(
    PCSTR pszFilePath,
    uid_t* pUid
    );

CENTERROR
CTCopyFileWithOriginalPerms(
    PCSTR pszSrcPath,
    PCSTR pszDstPath
    );

CENTERROR
CTChangePermissions(
    PCSTR pszPath,
    mode_t dwFileMode
    );

CENTERROR
CTGetOwnerAndPermissions(
    PCSTR pszSrcPath,
    uid_t * uid,
    gid_t * gid,
    mode_t * mode
    );
	
CENTERROR
CTGetFileTimeStamps(
	PCSTR pszFilePath,
	time_t *patime,
    time_t *pmtime,
    time_t *pctime );

CENTERROR
CTChangeOwner(
    PCSTR pszPath,
    uid_t uid,
    gid_t gid
    );

CENTERROR
CTChangeOwnerAndPermissions(
    PCSTR pszPath,
    uid_t uid,
    gid_t gid,
    mode_t dwFileMode
    );

/* Copies the file mode, owner, and group from the template file */
CENTERROR
CTCloneFilePerms(
    PCSTR pszTemplatePath,
    PCSTR pszDstPath
    );

CENTERROR
CTCreateDirectory(
    PCSTR pszPath,
    mode_t dwFileMode
    );

CENTERROR
CTCopyDirectory(
    PCSTR source,
    PCSTR dest
    );

/* Safely creates a temporary directory that is only accessible by the current
 * user and root.
 */
CENTERROR
CTCreateTempDirectory(
    PSTR *pszPath
    );

CENTERROR
CTGetMatchingFilePathsInFolder(
    PCSTR pszDirPath,
    PCSTR pszFileNameRegExp,
    PSTR** pppszHostFilePaths,
    PDWORD pdwNPaths
    );

CENTERROR
CTGetMatchingDirPathsInFolder(
    PCSTR pszDirPath,
    PCSTR pszDirNameRegExp,
    PSTR** pppszHostDirPaths,
    PDWORD pdwNPaths
    );

CENTERROR
CTCheckFileHoldsPattern(
    PCSTR pszFilePath,
    PCSTR pszPattern,
    PBOOLEAN pbPatternExists
    );

CENTERROR
CTFileContentsSame(
    PCSTR pszFilePath1,
    PCSTR pszFilePath2,
    PBOOLEAN pbSame
    );

CENTERROR
CTStreamContentsSame(
    FILE *fp1,
    FILE *fp2,
    PBOOLEAN pbSame
    );

CENTERROR
CTGetAbsolutePath(
    PSTR pszRelativePath,
    PSTR* ppszAbsolutePath
    );

CENTERROR
CTRemoveFiles(
    PSTR pszPath,
    BOOLEAN fDirectory,
    BOOLEAN fRecursive
    );

CENTERROR
CTBackupFile(
    PCSTR path
    );
	
CENTERROR
CTReadFile(
	PCSTR pszFilePath,
	PSTR *ppBuffer,
	PLONG pSize );

CENTERROR
CTOpenFile(
    PCSTR path,
    PCSTR mode, 
    FILE** handle);

CENTERROR
CTSetCloseOnExec(
    int fd);

CENTERROR
CTFileStreamWrite(
    FILE* handle,
    PCSTR data,
    unsigned int size);

CENTERROR
CTFilePrintf(
    FILE* handle,
    PCSTR format,
    ...
    );

//Use CTSafeCloseFile instead
CENTERROR
CTCloseFile(
    FILE* handle);

/* This function calls fclose on the handle and returns a centerror if
 * something goes wrong.
 *
 * One way this function could fail is if an IO error occurs while the file
 * buffers are being flushed, in which case the previous writes the file would
 * be incomplete.
 *
 * Even if fclose fails, the man fclose man page says that the handle is
 * invalid afterwards. This means that is an error to ever attempt to call
 * fclose on the same handle twice. That could result in a memory double free,
 * or segfault.
 *
 * To prevent the user from calling close twice, this function sets the handle
 * to NULL whether or not the close is successful.
 *
 * If this function is called with *handle == NULL, it will simply return
 * success.
 */
CENTERROR
CTSafeCloseFile(
    FILE** handle);

CENTERROR
CTMoveFileAcrossDevices(
    PCSTR pszSrcPath,
    PCSTR pszDstPath
    );

CENTERROR
CTGetCurrentDirectoryPath(
    PSTR* ppszPath
    );

/** This will run a sed expression on the src file and save the ouput into the dst file. It is safe to use the same path for the src and dst file names.

The dst file will be backed up before it is changed. After the sed operation finishes, the dst file will have the permissions of the src file.

The worst that can happen if this command is used to write to a user writeable directory, is that the function can fail if a user has already created the temporary file that this function uses.

So, that means if this function succeeds, a nonroot user did not tampered with the output file before or during the sed operation.
*/
CENTERROR
CTRunSedOnFile(
    PCSTR pszSrcPath,
    PCSTR pszDstPath,
    BOOLEAN bDashN,
    PCSTR pszExpression
    );

/* Sets changes to true if running sed on the source file with the given
 * expression outputs data different from the source file.
 */
CENTERROR
CTWillSedChangeFile(
    PCSTR pszSrcPath,
    PCSTR pszExpression,
    BOOLEAN *changes
    );

CENTERROR
CTFindSed(
        PSTR *sedPath
        );

/* Searches for a file or link inside of a colon separated search path.
 *
 * If the file is found, then the result is stored in foundPath. If multiple
 * paths are found, then the first one takes precedence. If the file is not
 * found, then CENTERROR_FILE_NOT_FOUND is returned, and foundPath is set to
 * NULL.
 *
 * It is the caller's responsibility to free foundPath.
 */
CENTERROR
CTFindFileInPath(
    PCSTR filename,
    PCSTR searchPath,
    PSTR* foundPath
    );

CENTERROR
CTFindInPath(
    PCSTR rootPrefix,
    PCSTR filename,
    PCSTR searchPath,
    PSTR* foundPath
    );

CENTERROR
CTGetFileTempPath(
    PCSTR unresolvedSrcPath,
    PSTR* resolvedSrcPath,
    PSTR* tempPath
    );

CENTERROR
CTGetFileDiff(
    PCSTR first,
    PCSTR second,
    PSTR *diff,
    BOOLEAN failIfNoDiff
    );

CENTERROR
CTReadNextLine(
    FILE* fp,
    PSTR *output,
    PBOOLEAN pbEndOfFile
    );

/* Reads a file and separates into individual lines
 * The dynamic array must be initialized (at least zeroed out) beforehand
 */
CENTERROR
CTReadLines(FILE *file, DynamicArray *dest);

CENTERROR
CTWriteLines(FILE *file, const DynamicArray *lines);

void
CTFreeLines(DynamicArray *lines);

LW_END_EXTERN_C


#endif /* __CTFILEUTILS_H__ */
