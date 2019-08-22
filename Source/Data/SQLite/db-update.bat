::*******************************************************************************************************
::  db-update.bat - Gbtc
::
::  Copyright © 2013, Grid Protection Alliance.  All Rights Reserved.
::
::  Licensed to the Grid Protection Alliance (GPA) under one or more contributor license agreements. See
::  the NOTICE file distributed with this work for additional information regarding copyright ownership.
::  The GPA licenses this file to you under the MIT License (MIT), the "License"; you may
::  not use this file except in compliance with the License. You may obtain a copy of the License at:
::
::      http://www.opensource.org/licenses/MIT
::
::  Unless agreed to in writing, the subject software distributed under the License is distributed on an
::  "AS-IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. Refer to the
::  License for the specific language governing permissions and limitations.
::
::  Code Modification History:
::  -----------------------------------------------------------------------------------------------------
::  07/28/2011 - Stephen C. Wills
::       Generated original version of source code.
::
::*******************************************************************************************************

@ECHO OFF

SETLOCAL EnableDelayedExpansion

::IF "%git%" == "" SET git=%PROGRAMFILES(X86)%\Git\cmd\git.exe

SET db[1]=PTPSync.db
SET db[2]=PTPSync-InitialDataSet.db
SET db[3]=PTPSync-SampleDataSet.db
SET script[1]=PTPSync.sql
SET script[2]=InitialDataSet.sql
SET script[3]=SampleDataSet.sql

FOR %%i IN (1 2 3) DO (
        ECHO Updating "!db[%%i]!"...
        
        FOR /f %%v IN ('CALL sqlite3 "!db[%%i]!" "PRAGMA user_version"') DO SET /a version=%%v+1
        
        IF "!prevdb!" == "" (
            DEL "!db[%%i]!"
        ) ELSE (
            COPY /Y "!prevdb!" "!db[%%i]!"
        )
        
        sqlite3 "!db[%%i]!" < "!script[%%i]!"
        sqlite3 "!db[%%i]!" "PRAGMA user_version = !version!"
    
    SET prevdb=!db[%%i]!
)