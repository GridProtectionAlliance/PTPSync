﻿using System.Reflection;
using System.Runtime.InteropServices;

// Assembly identity attributes.
[assembly: AssemblyVersion("1.1.184.0")]

// Informational attributes.
[assembly: AssemblyCompany("Grid Protection Alliance")]
[assembly: AssemblyCopyright("Copyright © 2015.  All Rights Reserved.")]
[assembly: AssemblyProduct("PTPSync")]

// Assembly manifest attributes.
#if DEBUG
[assembly: AssemblyConfiguration("Debug Build")]
#else
[assembly: AssemblyConfiguration("Release Build")]
#endif
[assembly: AssemblyDescription("Web services used by PTPSync.")]
[assembly: AssemblyTitle("PTPSync Web Services")]

// Other configuration attributes.
[assembly: ComVisible(false)]
[assembly: Guid("08996eaa-cea3-492f-b3d4-12af2277f17c")]
