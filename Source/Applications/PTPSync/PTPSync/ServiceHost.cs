//******************************************************************************************************
//  ServiceHost.cs - Gbtc
//
//  Copyright © 2015, Grid Protection Alliance.  All Rights Reserved.
//
//  Licensed to the Grid Protection Alliance (GPA) under one or more contributor license agreements. See
//  the NOTICE file distributed with this work for additional information regarding copyright ownership.
//  The GPA licenses this file to you under the Eclipse Public License -v 1.0 (the "License"); you may
//  not use this file except in compliance with the License. You may obtain a copy of the License at:
//
//      http://www.opensource.org/licenses/eclipse-1.0.php
//
//  Unless agreed to in writing, the subject software distributed under the License is distributed on an
//  "AS-IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. Refer to the
//  License for the specific language governing permissions and limitations.
//
//  Code Modification History:
//  ----------------------------------------------------------------------------------------------------
//  09/02/2009 - J. Ritchie Carroll
//       Generated original version of source code.
//
//******************************************************************************************************

using GSF;
using GSF.Configuration;
using GSF.Data;
using GSF.Data.Model;
using GSF.Diagnostics;
using GSF.IO;
using GSF.TimeSeries;
using System;
using System.IO;
using System.Linq;
using System.Net.NetworkInformation;
using System.Reflection;

// ReSharper disable once EmptyGeneralCatchClause
// ReSharper disable once UnusedMember.Local
// ReSharper disable UnusedParameter.Local
namespace PTPSync
{
    public class ServiceHost : ServiceHostBase
    {
        public ServiceHost()
        {
            ServiceName = "PTPSync";
            SetupRequiredAdapters();
        }

        protected override void ServiceStartingHandler(object sender, EventArgs<string[]> e)
        {
            // Handle base class service starting procedures
            base.ServiceStartingHandler(sender, e);

            // Make sure PTPSync specific default service settings exist
            CategorizedSettingsElementCollection systemSettings = ConfigurationFile.Current.Settings["systemSettings"];

            systemSettings.Add("CompanyName", "Grid Protection Alliance", "The name of the company who owns this instance of the PTPSync.");
            systemSettings.Add("CompanyAcronym", "GPA", "The acronym representing the company who owns this instance of the PTPSync.");
        }

        private void SetupRequiredAdapters()
        {
            try
            {
                const string PTPdProcessName = "PTPD!PROCESS";
                const string StatisticServicesName = "STATISTIC!SERVICES";
                const string DataPublisherName = "INTERNAL!DATAPUBLISHER";

                // Access needed settings from specified categories in configuration file
                CategorizedSettingsElementCollection systemSettings = ConfigurationFile.Current.Settings["systemSettings"];
                string newNodeID = Guid.NewGuid().ToString();

                // Make sure needed settings exist
                systemSettings.Add("NodeID", newNodeID, "Unique Node ID");

                // Get settings as currently defined in configuration file
                Guid nodeID = Guid.Parse(systemSettings["NodeID"].ValueAs(newNodeID));

                // Open database connection as defined in configuration file "systemSettings" category
                using (AdoDataConnection connection = new AdoDataConnection("systemSettings"))
                {
                    // Make sure Grafana process adapter exists
                    TableOperations<CustomActionAdapter> actionAdapterTable = new TableOperations<CustomActionAdapter>(connection);

                    // Make sure Grafana process adapter exists
                    CustomActionAdapter actionAdapter = actionAdapterTable.QueryRecordWhere("AdapterName = {0}", PTPdProcessName) ?? actionAdapterTable.NewRecord();

                    // Update record fields
                    actionAdapter.NodeID = nodeID;
                    actionAdapter.AdapterName = PTPdProcessName;
                    actionAdapter.AssemblyName = "FileAdapters.dll";
                    actionAdapter.TypeName = "FileAdapters.ProcessLauncher";
                    actionAdapter.Enabled = true;

                    string interfaceGuid = null;

                    // Scan network interfaces
                    try
                    {
                        NetworkInterface[] networkInterfaces = NetworkInterface.GetAllNetworkInterfaces().Where(ni => 
                            ni.OperationalStatus == OperationalStatus.Up &&
                            ni.NetworkInterfaceType != NetworkInterfaceType.Loopback &&
                            ni.NetworkInterfaceType != NetworkInterfaceType.Tunnel).ToArray();

                        if (networkInterfaces.Length > 0)
                        {
                            interfaceGuid = networkInterfaces[0].Id;
                            File.WriteAllText(FilePath.GetAbsolutePath("NetworkInterfaces.txt"), $"Local Network Interfaces:\r\n\r\n{string.Join("\r\n", networkInterfaces.Select(ni => $"{ni.Name} - {ni.Description}: {ni.Id}"))}");
                        }
                    }
                    catch 
                    {
                    }

                    // Define default adapter connection string if none is defined
                    if (string.IsNullOrWhiteSpace(actionAdapter.ConnectionString))
                        actionAdapter.ConnectionString = $"FileName=ptpd.exe; Arguments={{-b {interfaceGuid ?? "{InterfaceGuidHere}"} -V -g}}; ForceKillOnDispose=True";

                    // Save record updates
                    actionAdapterTable.AddNewOrUpdateRecord(actionAdapter);

                    // Make sure statistic services adapter exists
                    actionAdapter = actionAdapterTable.QueryRecordWhere("AdapterName = {0}", StatisticServicesName) ?? actionAdapterTable.NewRecord();

                    // Update record fields
                    actionAdapter.NodeID = nodeID;
                    actionAdapter.AdapterName = StatisticServicesName;
                    actionAdapter.AssemblyName = "GSF.TimeSeries.dll";
                    actionAdapter.TypeName = "GSF.TimeSeries.Statistics.StatisticsEngine";
                    actionAdapter.Enabled = true;

                    // Save record updates
                    actionAdapterTable.AddNewOrUpdateRecord(actionAdapter);

                    // Make sure internal data publisher adapter exists
                    actionAdapter = actionAdapterTable.QueryRecordWhere("AdapterName = {0}", DataPublisherName) ?? actionAdapterTable.NewRecord();

                    // Update record fields
                    actionAdapter.NodeID = nodeID;
                    actionAdapter.AdapterName = DataPublisherName;
                    actionAdapter.AssemblyName = "GSF.TimeSeries.dll";
                    actionAdapter.TypeName = "GSF.TimeSeries.Transport.DataPublisher";
                    actionAdapter.ConnectionString = "securityMode=None; allowSynchronizedSubscription=false; useBaseTimeOffsets=true; cacheMeasurementKeys={FILTER ActiveMeasurements WHERE SignalType = 'STAT'}";
                    actionAdapter.Enabled = true;

                    // Save record updates
                    actionAdapterTable.AddNewOrUpdateRecord(actionAdapter);
                }
            }
            catch (Exception ex)
            {
                LogPublisher log = Logger.CreatePublisher(typeof(ServiceHost), MessageClass.Application);
                log.Publish(MessageLevel.Error, "Error Message", "Failed to setup PTPd hosting adapter", null, ex);
            }
        }

        private static void KeyStartupOperations(AdoDataConnection database, string nodeIDQueryString, ulong trackingVersion, string arguments, Action<string> statusMessage, Action<Exception> processException)
        {
            const BindingFlags DataOperationFlags = BindingFlags.NonPublic | BindingFlags.Static;
            Type commonStartupOperations = typeof(TimeSeriesStartupOperations);

            commonStartupOperations.GetMethod("ValidateDefaultNode", DataOperationFlags)?.Invoke(null, new object[] { database, nodeIDQueryString });
            commonStartupOperations.GetMethod("ValidateAdapterCollections", DataOperationFlags)?.Invoke(null, new object[] { database, nodeIDQueryString });
            commonStartupOperations.GetMethod("ValidateActiveMeasurements", DataOperationFlags)?.Invoke(null, new object[] { database, nodeIDQueryString });
            commonStartupOperations.GetMethod("ValidateAccountsAndGroups", DataOperationFlags)?.Invoke(null, new object[] { database, nodeIDQueryString });
            commonStartupOperations.GetMethod("ValidateStatistics", DataOperationFlags)?.Invoke(null, new object[] { database, nodeIDQueryString });
        }
    }
}
