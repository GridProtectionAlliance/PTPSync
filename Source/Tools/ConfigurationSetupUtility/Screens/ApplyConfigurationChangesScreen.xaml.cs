//******************************************************************************************************
//  ApplyConfigurationChangesScreen.xaml.cs - Gbtc
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
//  10/12/2010 - Stephen C. Wills
//       Generated original version of source code.
//
//******************************************************************************************************

using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Windows;

namespace ConfigurationSetupUtility.Screens
{
    /// <summary>
    /// Interaction logic for ApplyConfigurationChangesScreen.xaml
    /// </summary>
    public partial class ApplyConfigurationChangesScreen : IScreen
    {
        #region [ Members ]

        // Fields

        private NodeSelectionScreen m_nodeSelectionScreen;
        private HistorianSetupScreen m_historianSetupScreen;
        private SetupReadyScreen m_setupReadyScreen;
        private Dictionary<string, object> m_state;

        #endregion

        #region [ Constructors ]

        /// <summary>
        /// Creates a new instance of the <see cref="ApplyConfigurationChangesScreen"/> class.
        /// </summary>
        public ApplyConfigurationChangesScreen()
        {
            InitializeComponent();
        }

        #endregion

        #region [ Properties ]

        /// <summary>
        /// Gets the screen to be displayed when the user clicks the "Next" button.
        /// </summary>
        public IScreen NextScreen
        {
            get
            {
                bool applyChangesToService = Convert.ToBoolean(m_state["applyChangesToService"]);
                bool applyChangesToLocalManager = Convert.ToBoolean(m_state["applyChangesToLocalManager"]);
                bool existing = Convert.ToBoolean(m_state["existing"]);
                bool setupHistorian = Convert.ToBoolean(m_state["setupHistorian"]);

                if (setupHistorian)
                    return m_historianSetupScreen;

                if (existing && (applyChangesToService || applyChangesToLocalManager))
                    return m_nodeSelectionScreen;

                return m_setupReadyScreen;
            }
        }

        /// <summary>
        /// Gets a boolean indicating whether the user can advance to
        /// the next screen from the current screen.
        /// </summary>
        public bool CanGoForward
        {
            get
            {
                return true;
            }
        }

        /// <summary>
        /// Gets a boolean indicating whether the user can return to
        /// the previous screen from the current screen.
        /// </summary>
        public bool CanGoBack
        {
            get
            {
                return true;
            }
        }

        /// <summary>
        /// Gets a boolean indicating whether the user can cancel the
        /// setup process from the current screen.
        /// </summary>
        public bool CanCancel
        {
            get
            {
                return true;
            }
        }

        /// <summary>
        /// Gets a boolean indicating whether the user input is valid on the current page.
        /// </summary>
        public bool UserInputIsValid
        {
            get
            {
                return true;
            }
        }

        /// <summary>
        /// Collection shared among screens that represents the state of the setup.
        /// </summary>
        public Dictionary<string, object> State
        {
            get
            {
                return m_state;
            }
            set
            {
                m_state = value;
                InitializeState();
            }
        }

        /// <summary>
        /// Allows the screen to update the navigation buttons after a change is made
        /// that would affect the user's ability to navigate to other screens.
        /// </summary>
        public Action UpdateNavigation { get; set; }

        #endregion

        #region [ Methods ]

        // Initializes the state keys to their default values.
        private void InitializeState()
        {
            object webManagerDir = Registry.GetValue("HKEY_LOCAL_MACHINE\\Software\\PTPSyncManagerServices", "Installation Path", null) ?? Registry.GetValue("HKEY_LOCAL_MACHINE\\Software\\Wow6432Node\\PTPSyncManagerServices", "Installation Path", null);
            bool managerOptionsEnabled = m_state["configurationType"].ToString() == "database";
            bool webManagerOptionEnabled = managerOptionsEnabled && (webManagerDir != null);
            bool existing = Convert.ToBoolean(m_state["existing"]);
            bool initialDataScript = !existing && Convert.ToBoolean(m_state["initialDataScript"]);

            m_nodeSelectionScreen = new NodeSelectionScreen();

            if (m_state != null && m_state.ContainsKey("setupReadyScreen"))
            {
                m_setupReadyScreen = (SetupReadyScreen)m_state["setupReadyScreen"];
            }
            else
            {
                m_setupReadyScreen = new SetupReadyScreen();
                m_state["setupReadyScreen"] = m_setupReadyScreen;
            }

            if (m_state != null && m_state.ContainsKey("historianSetupScreen"))
            {
                m_historianSetupScreen = (HistorianSetupScreen)m_state["historianSetupScreen"];
            }
            else
            {
                m_historianSetupScreen = new HistorianSetupScreen();
                m_state["historianSetupScreen"] = m_historianSetupScreen;
            }

            // Enable or disable the options based on whether those options are available for the current configuration.
            m_PTPSyncManagerLocalCheckBox.IsEnabled = managerOptionsEnabled;
            m_PTPSyncManagerWebCheckBox.IsEnabled = webManagerOptionEnabled;

            // If the options are disabled, they must also be unchecked.
            if (!managerOptionsEnabled)
                m_PTPSyncManagerLocalCheckBox.IsChecked = false;

            if (!webManagerOptionEnabled)
                m_PTPSyncManagerWebCheckBox.IsChecked = false;

            m_setupHistorianCheckBox.IsChecked = false;

            // Set up the state object with the proper initial values.
            m_state["applyChangesToService"] = m_PTPSyncServiceCheckBox.IsChecked.GetValueOrDefault();
            m_state["applyChangesToLocalManager"] = m_PTPSyncManagerLocalCheckBox.IsChecked.GetValueOrDefault();
            m_state["applyChangesToWebManager"] = m_PTPSyncManagerWebCheckBox.IsChecked.GetValueOrDefault();
            //m_state["setupHistorian"] = initialDataScript;

            m_state["setupHistorian"] = (bool)m_setupHistorianCheckBox.IsChecked;
            m_setupHistorianCheckBox.Visibility = Visibility.Collapsed;
            m_PTPSyncManagerWebCheckBox.Visibility = Visibility.Collapsed;

            m_horizontalRule.Visibility = m_setupHistorianCheckBox.Visibility;
        }

        // Occurs when the user chooses to apply changes to the PTPSync service.
        private void PTPSyncServiceCheckBox_Checked(object sender, RoutedEventArgs e)
        {
            if (m_state != null)
                m_state["applyChangesToService"] = true;
        }

        // Occurs when the user chooses to not apply changes to the PTPSync service.
        private void PTPSyncServiceCheckBox_Unchecked(object sender, RoutedEventArgs e)
        {
            if (m_state != null)
                m_state["applyChangesToService"] = false;
        }

        // Occurs when the user chooses to changes to the local PTPSync Manager application.
        private void PTPSyncManagerLocalCheckBox_Checked(object sender, RoutedEventArgs e)
        {
            if (m_state != null)
                m_state["applyChangesToLocalManager"] = true;
        }

        // Occurs when the user chooses to not apply changes to the local PTPSync Manager application.
        private void PTPSyncManagerLocalCheckBox_Unchecked(object sender, RoutedEventArgs e)
        {
            if (m_state != null)
                m_state["applyChangesToLocalManager"] = false;
        }

        // Occurs when the user chooses to apply changes to the PTPSync Manager web application.
        private void PTPSyncManagerWebCheckBox_Checked(object sender, RoutedEventArgs e)
        {
            if (m_state != null)
                m_state["applyChangesToWebManager"] = true;
        }

        // Occurs when the user chooses to not apply changes to the PTPSync Manager web application.
        private void PTPSyncManagerWebCheckBox_Unchecked(object sender, RoutedEventArgs e)
        {
            if (m_state != null)
                m_state["applyChangesToWebManager"] = false;
        }

        // Occurs when the user chooses to setup historian.
        private void SetupHistorianCheckBox_Checked(object sender, RoutedEventArgs e)
        {
            if (m_state != null)
                m_state["setupHistorian"] = true;
        }

        // Occurs when the user chooses to not to setup historian.
        private void SetupHistorianCheckBox_Unchecked(object sender, RoutedEventArgs e)
        {
            if (m_state != null)
                m_state["setupHistorian"] = false;
        }
        #endregion
    }
}
