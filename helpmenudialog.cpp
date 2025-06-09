#include"helpmenudialog.h"
#include <QApplication>
#include"Constants.h"
#include<QString>

HelpMenuDialog::HelpMenuDialog(HelpType type, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(getTitle(type));
    setModal(true);
    resize(600, 500);

    setupUI();
    loadContent(type);
}

void HelpMenuDialog::setupUI()
{
    m_layout = new QVBoxLayout(this);

    m_contentArea = new QTextEdit(this);
    m_contentArea->setReadOnly(true);

    m_closeButton = new QPushButton("Close", this);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);

    m_layout->addWidget(m_contentArea);
    m_layout->addWidget(m_closeButton);
}

void HelpMenuDialog::loadContent(HelpType type)
{
    QString content;

    switch (type) {
    case HelpType::About:
        content = getAboutContent();
        break;
    case HelpType::Features:
        content = getFeaturesContent();
        break;
    case HelpType::Instructions:
        content = getInstructionsContent();
        break;
    case HelpType::BestPractices:
        content = getBestPracticesContent();
        break;
    case HelpType::WhatsNew:
        content = getWhatsNewContent();
        break;
    case HelpType::Shortcuts:
        content = getShortcutsContent();
        break;
    case HelpType::Security:
        content = getSecurityContent();
        break;
    case HelpType::TwoFA:
        content = getTwoFAContent();
        break;
    case HelpType::DataManagement:
        content = getDataManagementContent();
        break;
    case HelpType::DownloadManagement:
        content = getDownloadManagerContent();
        break;
    case HelpType::onSitesAndSessions:
        content = getSitesSessionsContent();
        break;
    case HelpType::onSecurity:
        content = getOnSecurityContent();
        break;
    case HelpType::onNewStorageSystem:
        content = getOnNewStorageSystemContent();
        break;
    }

    m_contentArea->setHtml(content);
}

QString HelpMenuDialog::getTitle(HelpType type)
{
    switch (type) {
    case HelpType::About: return "About Jasmine";
    case HelpType::Features: return "Features";
    case HelpType::Instructions: return "Instructions";
    case HelpType::BestPractices: return "Best Practices";
    case HelpType::WhatsNew: return "What's New";
    case HelpType::Shortcuts: return "Keyboard Shortcuts";
    case HelpType::Security: return "Jasmine Security";
    case HelpType::TwoFA: return "Jasmine 2FA Utility";
    case HelpType::DataManagement: return "Jasmine Data Management";
    case HelpType::DownloadManagement: return "Jasmine Download Management";
    case HelpType::onSitesAndSessions: return "On Sites And Sessions";
    case HelpType::onSecurity: return "On Security";
    case HelpType::onNewStorageSystem: return "On the New Storage System";

    default: return "Help";
    }
}

QString HelpMenuDialog::getBestPracticesContent()
{
    return QString();  // TODO: Implement
}

QString HelpMenuDialog::getWhatsNewContent()
{
    return QString();  // TODO: Implement
}

QString HelpMenuDialog::getShortcutsContent()
{
    return QString();  // TODO: Implement
}



QString HelpMenuDialog::getAboutContent() {
    return QString(R"(
        <div style="text-align: center; font-family: Arial, sans-serif;">
            <h1 style="color: #2c3e50; margin-bottom: 10px;">🌸 Jasmine</h1>
            <h3 style="color: #7f8c8d; margin-bottom: 20px;">Website & Session Manager</h3>
            <p style="font-size: 16px; margin-bottom: 20px;">
                A comprehensive web launcher and session management application that transforms
                your scattered bookmarks and browser tabs into an organized, launchable workspace.
            </p>
            <div style="background-color: #ecf0f1; padding: 15px; border-radius: 8px; margin: 20px 0;">
                <p style="margin: 5px 0;"><strong>Version:</strong> %1</p>
                <p style="margin: 5px 0;"><strong>Built with:</strong> Qt Framework</p>
                <p style="margin: 5px 0;"><strong>Platform:</strong> Cross-platform</p>
            </div>
            <div style="margin: 30px 0;">
                <h4 style="color: #2c3e50;">Key Features</h4>
                <p style="text-align: left; margin: 10px 20px;">
                    • Smart bookmarking with favicons and metadata<br>
                    • Multi-tab session management<br>
                    • Private profile isolation for multi-account support<br>
                    • Built-in download manager and screenshot capture<br>
                    • 2FA integration and login reference storage
                </p>
            </div>
            <hr style="margin: 30px 0; border: 1px solid #bdc3c7;">
            <div style="margin: 20px 0;">
                <p style="margin: 5px 0; color: #7f8c8d;">
                    <strong>Copyright © 2025 Alamahant</strong>
                </p>
                <p style="margin: 5px 0; font-size: 12px; color: #95a5a6;">
                    All rights reserved. This software is provided as-is without warranty.
                </p>
            </div>
            <div style="margin: 20px 0;">
                <p style="font-size: 14px; color: #7f8c8d;">
                    Made with ❤️ for productivity enthusiasts and multi-account managers
                </p>
            </div>
        </div>
    )").arg(APP_VERSION);
}

QString HelpMenuDialog::getFeaturesContent()
{
    return R"(
        <div style="font-family: Arial, sans-serif; line-height: 1.6; color: #2c3e50;">
            <h1 style="color: #e74c3c; text-align: center; margin-bottom: 20px;">🌸 Jasmine Features</h1>

            <div style="background-color: #ecf0f1; padding: 15px; border-radius: 8px; margin-bottom: 25px;">
                <h2 style="color: #2c3e50; margin-top: 0;">Website & Session Manager</h2>
                <p>A comprehensive web launcher and session management application that combines bookmarking,
                multi-tab session handling, flexible browsing profiles, and integrated web utilities into one streamlined tool.</p>
            </div>

            <h2 style="color: #3498db; border-bottom: 2px solid #3498db; padding-bottom: 5px;">🚀 Core Features</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #e67e22;">📚 Smart Bookmarking</h3>
                <p>Store websites with titles, URLs, comments, favicons, and login references for complete organization.</p>

                <h3 style="color: #e67e22;">💾 Session Management</h3>
                <p>Create, save, and restore multi-tab browsing sessions from your saved sites with one click.</p>

                <h3 style="color: #e67e22;">🔒 Flexible Web Profiles</h3>
                <p>Toggle between isolated private profiles per site or shared profiles across sites for maximum control.</p>

                <h3 style="color: #e67e22;">👥 Multi-Account Support</h3>
                <p>Simultaneously access multiple accounts on the same service (Gmail, GitHub, ProtonMail, etc.) without conflicts.</p>

                <h3 style="color: #e67e22;">⚡ Launch Control</h3>
                <p>Quick-launch individual websites or entire browsing sessions instantly.</p>
                <h3 style="color: #e67e22;">📥 Fully Functional Download Manager</h3>
                <p>Jasmine implements a complete download management system that handles all file downloads directly within the application. Track download progress, manage completed downloads, or cancel downloads. Jasmine uses &lt;UserHomeDirectory&gt;/Downloads/Jasmine as the default download location.</p>

                <h3 style="color: #e67e22;">📸 Screenshot Capture</h3>
                <p>Take and save screenshots of web pages for documentation or reference.</p>

                <h3 style="color: #e67e22;">🔑 Login Reference</h3>
                <p>Store username/password reminders for easy reference with privacy controls.</p>

                <h3 style="color: #e67e22;">🔐 2FA Integration</h3>
                <p>Built-in two-factor authentication code generator for enhanced security.</p>

                <h3 style="color: #e67e22;">📊 Visit Tracking</h3>
                <p>Monitor site usage with visit counts and last-accessed timestamps.</p>
            </div>

            <h2 style="color: #9b59b6; border-bottom: 2px solid #9b59b6; padding-bottom: 5px;">🎯 Perfect For</h2>
            <ul style="margin: 15px 0; padding-left: 20px;">
                <li>Managing multiple accounts on the same service with isolated sessions</li>
                <li>Users who work with multiple web applications daily and need integrated utilities</li>
                <li>Developers managing various development/staging environments with documentation needs</li>
                <li>Anyone who wants organized access to frequently-used sites with full session control</li>
                <li>Teams needing quick access to shared web resources with built-in productivity tools</li>
                <li>Content creators managing multiple channels and social media accounts</li>
                <li>Freelancers working with multiple clients requiring data isolation</li>
            </ul>

            <h2 style="color: #e74c3c; border-bottom: 2px solid #e74c3c; padding-bottom: 5px;">🔐 Private Profile System</h2>

            <div style="background-color: #fff3cd; padding: 15px; border-radius: 8px; margin: 15px 0;">
                <h3 style="color: #856404; margin-top: 0;">What Are Private Profiles?</h3>
                <p style="margin-bottom: 0;">Private profiles are completely isolated browsing environments within Jasmine.
                Each operates as a separate browser with its own cookies, history, cache, passwords, and preferences.</p>
            </div>

            <h3 style="color: #27ae60;">🕵️ Incognito Mode (Temporary)</h3>
            <p>Launch websites with private profiles without saving - perfect for testing, temporary access,
            or sensitive browsing. All data is deleted when you close the tab.</p>

            <h3 style="color: #27ae60;">💼 Persistent Multi-Account Management</h3>
            <p>Save private profile sessions to maintain permanent isolated environments for each account.
            Perfect for managing multiple Gmail, GitHub, or social media accounts simultaneously.</p>

            <h2 style="color: #f39c12; border-bottom: 2px solid #f39c12; padding-bottom: 5px;">💡 Real-World Examples</h2>

            <div style="margin: 15px 0;">
                <h4 style="color: #d35400;">📧 Gmail Multi-Account Setup</h4>
                <p>• Personal Gmail (private profile)<br>
                • Work Gmail (private profile)<br>
                • Side project Gmail (private profile)<br>
                <strong>Result:</strong> Access all accounts simultaneously without conflicts!</p>

                <h4 style="color: #d35400;">💻 Developer Workflow</h4>
                <p>• Personal GitHub (private profile)<br>
                • Work GitHub (private profile)<br>
                • Open source contributions (private profile)<br>
                <strong>Result:</strong> Switch between coding identities instantly!</p>

                <h4 style="color: #d35400;">📱 Social Media Management</h4>
                <p>• Personal Twitter/Instagram/Facebook (private profiles)<br>
                • Business accounts (private profiles)<br>
                <strong>Result:</strong> Manage all accounts without constant login/logout!</p>
            </div>

            <h2 style="color: #1abc9c; border-bottom: 2px solid #1abc9c; padding-bottom: 5px;">🎨 Mix & Match Flexibility</h2>
            <p>Combine private profiles (isolation) with shared profiles (integration) in the same session:</p>
            <ul style="margin: 10px 0; padding-left: 20px;">
                <li><strong>Work Email:</strong> Private profile (isolated)</li>
                <li><strong>Calendar:</strong> Shared profile (can integrate with email)</li>
                <li><strong>Team Chat:</strong> Private profile (work-only)</li>
                <li><strong>Research:</strong> Shared profile (general browsing)</li>
            </ul>

            <div style="background-color: #d1ecf1; padding: 15px; border-radius: 8px; margin: 25px 0;">
                <h3 style="color: #0c5460; margin-top: 0;">🌟 The Jasmine Advantage</h3>
                <p style="margin-bottom: 0;"><strong>Transform your scattered bookmarks and browser tabs into an organized,
                launchable workspace with complete control over session isolation, account management, and integrated web utilities.</strong></p>
            </div>

            <div style="text-align: center; margin: 30px 0; padding: 20px; background-color: #f8f9fa; border-radius: 8px;">
                <h3 style="color: #495057; margin-top: 0;">🚀 Unprecedented Control</h3>
                <p style="margin-bottom: 0;">Jasmine gives you flexibility impossible with traditional browsers -
                be completely anonymous, maintain multiple persistent identities, or mix approaches based on your specific needs.</p>
            </div>
        </div>
    )";
}

QString HelpMenuDialog::getInstructionsContent()
{
    return R"(
        <div style="font-family: Arial, sans-serif; line-height: 1.6; color: #2c3e50;">
            <h1 style="color: #e74c3c; text-align: center; margin-bottom: 20px;">📋 How to Use Jasmine</h1>

            <div style="background-color: #ecf0f1; padding: 15px; border-radius: 8px; margin-bottom: 25px;">
                <h2 style="color: #2c3e50; margin-top: 0;">🏗️ Step 1: Create & Save Websites</h2>
                <p><strong>Websites are your building blocks.</strong> Create one entry per service - you'll launch it multiple times later.</p>

                <h3 style="color: #27ae60;">Adding Websites:</h3>
                <ol style="margin: 10px 0; padding-left: 25px;">
                    <li>Fill up the fields in the details panel (title, URL, comments, etc.)</li>
                    <li>Click <strong>"Add Website"</strong></li>
                    <li>Website is saved and appears as a card</li>
                </ol>

                <p>Create your library: Gmail, GitHub, ProtonMail, Slack, etc. One entry per service.</p>
            </div>

            <h2 style="color: #3498db; border-bottom: 2px solid #3498db; padding-bottom: 5px;">🚀 Step 2: Launch Websites to Create Tabs</h2>

            <div style="background-color: #fff3cd; padding: 15px; border-radius: 8px; margin: 15px 0;">
                <h3 style="color: #856404; margin-top: 0;">🎛️ The Magic Happens at Launch Time</h3>
                <p>You can launch websites in two ways:</p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Click the website card to launch it</li>
                    <li>Use the "Launch" button in the details panel</li>
                </ul>

                <p>Before launching, you'll see the "Use Private Profile" button on the toolbar.</p>

                <p><strong>Toggle Button (Press to switch):</strong></p>
                <p><strong>Button OFF:</strong> Tab uses shared profile (can see data from other shared tabs)</p>
                <p><strong>Button ON:</strong> Tab gets completely isolated profile (separate login, cookies, everything)</p>

                <p><strong>Key Point:</strong> You can launch the SAME website multiple times with different toggle settings!</p>
            </div>

            <h2 style="color: #9b59b6; border-bottom: 2px solid #9b59b6; padding-bottom: 5px;">📧 Example: Multiple Gmail Accounts</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #8e44ad;">The Process:</h3>

                <h4 style="color: #d35400;">1. Create ONE Gmail Website:</h4>
                <p>Fill details panel with "Gmail" and https://gmail.com → Click "Add Website"</p>

                <h4 style="color: #d35400;">2. Launch Gmail Multiple Times:</h4>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Select "Gmail" → Toggle button ON → Launch → Log into personal account</li>
                    <li>Select "Gmail" → Toggle button ON → Launch → Log into work account</li>
                    <li>Select "Gmail" → Toggle button ON → Launch → Log into side project account</li>
                    <li>Launch as many as you need...</li>
                </ul>

                <h4 style="color: #d35400;">3. Save Your Tab Collection:</h4>
                <p>Sessions tab → "Save Current Session" → Name: "All Gmail Accounts"</p>

                <div style="background-color: #d4edda; padding: 12px; border-radius: 6px; margin: 15px 0;">
                    <p style="margin: 0;"><strong>🎉 Result:</strong> Multiple Gmail tabs, each completely isolated, each logged into different accounts!</p>
                </div>
            </div>

            <h2 style="color: #e74c3c; border-bottom: 2px solid #e74c3c; padding-bottom: 5px;">💻 Example: Multiple GitHub Accounts</h2>

            <div style="margin: 20px 0;">
                <h4 style="color: #c0392b;">Same process, different service:</h4>
                <ol style="margin: 10px 0; padding-left: 25px;">
                    <li>Create "GitHub" website → Save</li>
                    <li>Launch GitHub multiple times (Toggle button ON each time)</li>
                    <li>Log into different GitHub accounts in each tab</li>
                    <li>Save as "All GitHub Accounts" session</li>
                </ol>

                <p><strong>Use Cases:</strong> Personal repos, work orgs, client projects, open source contributions, testing accounts, etc.</p>
            </div>

            <h2 style="color: #f39c12; border-bottom: 2px solid #f39c12; padding-bottom: 5px;">🎭 Mixed Website Sessions</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #e67e22;">Example: "Social Media Management" Session</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Launch Twitter multiple times (Toggle ON) → Different Twitter accounts</li>
                    <li>Launch Instagram multiple times (Toggle ON) → Different Instagram accounts</li>
                    <li>Launch Facebook multiple times (Toggle ON) → Different Facebook accounts</li>
                    <li>Launch LinkedIn twice (Toggle ON) → Personal + business LinkedIn</li>
                    <li>Launch YouTube multiple times (Toggle ON) → Different channels</li>
                </ul>
                <p><strong>Result:</strong> Multiple tabs across different platforms, all isolated accounts!</p>

                <h3 style="color: #e67e22;">Example: "Development Workflow" Session</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Launch GitHub multiple times (Toggle ON) → Personal, work, client repos</li>
                    <li>Launch Gmail multiple times (Toggle ON) → Work email, client emails</li>
                    <li>Launch Slack multiple times (Toggle ON) → Different team workspaces</li>
                    <li>Launch Stack Overflow once (Toggle OFF) → Shared research</li>
                    <li>Launch Documentation sites (Toggle OFF) → Shared browsing</li>
                </ul>
                <p><strong>Result:</strong> Multiple tabs mixing isolated accounts with shared research!</p>
            </div>

            <h2 style="color: #1abc9c; border-bottom: 2px solid #1abc9c; padding-bottom: 5px;">🎯 Smart Toggle Usage</h2>

            <div style="margin: 20px 0;">
                <h4 style="color: #148f77;">When to Use Toggle ON (Private):</h4>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Multiple accounts on same service (Gmail, GitHub, social media)</li>
                    <li>Client work that needs isolation</li>
                    <li>Testing different user roles</li>
                    <li>Sensitive accounts (banking, admin panels)</li>
                </ul>

                <h4 style="color: #148f77;">When to Use Toggle OFF (Shared):</h4>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Research and documentation browsing</li>
                    <li>Services that should integrate (Google Calendar with Gmail)</li>
                    <li>General browsing within same workflow</li>
                    <li>Single account services you don't need isolated</li>
                </ul>
            </div>

            <h2 style="color: #e74c3c; border-bottom: 2px solid #e74c3c; padding-bottom: 5px;">💾 Step 3: Save or Don't Save</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #c0392b;">🔥 Incognito Mode (Don't Save)</h3>
                <p>Launch websites with Toggle ON, use them, but DON'T save as session:</p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Perfect for testing, temporary access, one-time tasks</li>
                    <li>All private profile data deleted when you close Jasmine</li>
                    <li>Next time you launch = completely fresh</li>
                </ul>

                <h3 style="color: #c0392b;">💾 Persistent Sessions (Save)</h3>
                <p>Launch websites, build your tab collection, then save as session:</p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>All login states, cookies, and data preserved forever</li>
                    <li>Session appears as a card in Sessions container with details panel</li>
                    <li>One-click restoration of entire multi-account setup</li>
                </ul>
            </div>

            <h2 style="color: #9b59b6; border-bottom: 2px solid #9b59b6; padding-bottom: 5px;">🔄 Advanced: Working with Sessions</h2>

            <div style="margin: 20px 0;">
                <p>Sessions also have cards and details panels. Sessions are launchable just like websites.</p>

                <h3 style="color: #8e44ad;">Launching Sessions:</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Click session card to launch it</li>
                    <li>Use "Launch" button in session details panel</li>
                </ul>

                <h3 style="color: #8e44ad;">Session Launch Behavior:</h3>
                <p>If you already have tabs open and attempt to launch a session, a message box will ask:</p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>"Append" - Add session tabs to existing tabs</li>
                    <li>"Launch after deleting" - Close existing tabs first, then launch session</li>
                </ul>
            </div>

            <h2 style="color: #f39c12; border-bottom: 2px solid #f39c12; padding-bottom: 5px;">🔧 Expanding Existing Sessions</h2>

            <div style="background-color: #f3e5f5; padding: 15px; border-radius: 8px; margin: 20px 0;">
                <h3 style="color: #7b1fa2; margin-top: 0;">Pro Workflow:</h3>
                <ol style="margin: 10px 0; padding-left: 25px;">
                    <li><strong>Launch existing session</strong> → Restores all saved tabs</li>
                    <li><strong>Launch more websites</strong> → Toggle ON/OFF for new tabs</li>
                    <li><strong>Save updated session</strong> → Now includes original + new tabs</li>
                </ol>

                <p><strong>Example:</strong> Start with "Work Session", add more Gmail accounts (Toggle ON), save as updated session → Now includes original tabs + new isolated accounts!</p>

                <p><strong>Note:</strong> Toggle button does nothing when launching sessions - they restore with original toggle states. But you can add NEW tabs with toggle choices!</p>
            </div>

            <h2 style="color: #1abc9c; border-bottom: 2px solid #1abc9c; padding-bottom: 5px;">🌟 Real-World Use Cases</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #16a085;">🏢 Business Manager:</h3>
                <p>"Client Management" session:</p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Multiple Gmail accounts (one per client) - all Toggle ON</li>
                    <li>Multiple Slack workspaces (different teams) - all Toggle ON</li>
                    <li>Multiple GitHub accounts (different projects) - all Toggle ON</li>
                    <li>Multiple social media accounts (client brands) - all Toggle ON</li>
                    <li>Documentation sites - all Toggle OFF (shared research)</li>
                </ul>

                <h3 style="color: #16a085;">🎬 Content Creator:</h3>
                <p>"Content Empire" session:</p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>YouTube: Multiple channels (Toggle ON each)</li>
                    <li>Twitter: Multiple accounts (Toggle ON each)</li>
                    <li>Instagram: Multiple accounts (Toggle ON each)</li>
                    <li>TikTok: Multiple accounts (Toggle ON each)</li>
                    <li>Analytics tools: Toggle OFF (shared data)</li>
                </ul>

                <h3 style="color: #16a085;">💻 Developer:</h3>
                <p>"All Projects" session:</p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>GitHub: Multiple accounts (personal, work, clients) - Toggle ON</li>
                    <li>Gmail: Multiple accounts (different projects) - Toggle ON</li>
                    <li>AWS/Cloud consoles: Multiple accounts - Toggle ON</li>
                    <li>Documentation: Multiple tabs - Toggle OFF</li>
                    <li>Stack Overflow, forums: Toggle OFF</li>
                </ul>
            </div>

            <h2 style="color: #e67e22; border-bottom: 2px solid #e67e22; padding-bottom: 5px;">🎛️ Interface Controls</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #d35400;">Switch to WebView / Switch to Dashboard Toggle Button:</h3>
                <p>Use this button to switch between the dashboard view (where you manage websites and sessions) and the webview (where you browse your launched tabs). Press once to switch to webview, press again to return to dashboard.</p>

                <h3 style="color: #d35400;">Dark Theme / Light Theme Toggle Button:</h3>
                <p>Switch between dark and light visual themes for the interface. Press the button to toggle between themes based on your preference or lighting conditions.</p>
            </div>
            <div style="background-color: #ffebee; border: 2px solid #f44336; padding: 20px; border-radius: 8px; margin-bottom: 25px;">
                <h2 style="color: #d32f2f; margin-top: 0; text-align: center;">⚠️ IMPORTANT NOTICE</h2>
                <p style="font-weight: bold; color: #c62828; font-size: 16px; text-align: center;">
                In multi-tab sessions created with private profiles: If you wish to retain the login status,
                <strong style="color: #b71c1c;">DO NOT LOG OUT</strong> before saving the session!
                </p>
                <p style="color: #d32f2f; text-align: center; margin-bottom: 0;">
                Logging out will clear your authentication data and you'll need to log in again when restoring the session.
                </p>
            </div>


            <div style="text-align: center; margin: 30px 0; padding: 20px; background-color: #f8f9fa; border-radius: 8px;">
                <h3 style="color: #495057; margin-top: 0;">🚀 The Power of Scale</h3>
                <p style="margin-bottom: 0;">With Jasmine, you can manage multiple accounts across multiple services in organized, launchable sessions. The toggle button gives you precise control over isolation vs integration for each tab.</p>
            </div>
        </div>
    )";
}

QString HelpMenuDialog::getSecurityContent()
{
    return R"(
        <div style="font-family: Arial, sans-serif; line-height: 1.6; color: #2c3e50;">
            <h1 style="color: #e74c3c; text-align: center; margin-bottom: 20px;">🔐 Security Features</h1>

            <div style="background-color: #ecf0f1; padding: 15px; border-radius: 8px; margin-bottom: 25px;">
                <h2 style="color: #2c3e50; margin-top: 0;">Master Password Protection</h2>
                <p>Jasmine includes a comprehensive security system to protect your saved websites, sessions, and sensitive data.</p>
            </div>

            <h2 style="color: #3498db; border-bottom: 2px solid #3498db; padding-bottom: 5px;">🛡️ Password Protection Features</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Master password requirement on startup</li>
                    <li>Secure password hashing with salt encryption</li>
                    <li>Failed attempt protection (5 attempts maximum)</li>
                    <li>Factory reset option for forgotten passwords</li>
                </ul>
            </div>

            <h2 style="color: #27ae60; border-bottom: 2px solid #27ae60; padding-bottom: 5px;">🔧 How to Enable Password Protection</h2>

            <div style="margin: 20px 0;">
                <ol style="margin: 10px 0; padding-left: 25px;">
                    <li>Go to Security menu → "Require Password on Startup"</li>
                    <li>Read the security notice and click OK</li>
                    <li>Enter your new master password</li>
                    <li>Confirm your password</li>
                    <li>Jasmine will now require this password on every startup</li>
                </ol>
            </div>

            <div style="background-color: #fff3cd; padding: 15px; border-radius: 8px; margin: 15px 0;">
                <h3 style="color: #856404; margin-top: 0;">⚠️ Important Security Notes</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Choose a strong, memorable password</li>
                    <li>Write it down in a safe place</li>
                    <li>If you forget it, you'll need to factory reset</li>
                    <li>Password is encrypted and stored securely</li>
                </ul>
            </div>

            <h2 style="color: #f39c12; border-bottom: 2px solid #f39c12; padding-bottom: 5px;">🔄 Changing Your Master Password</h2>

            <div style="margin: 20px 0;">
                <ol style="margin: 10px 0; padding-left: 25px;">
                    <li>Go to Security menu → "Change Master Password"</li>
                    <li>Enter your new password</li>
                    <li>Confirm the new password</li>
                    <li>Password is updated immediately</li>
                </ol>
            </div>

            <h2 style="color: #e74c3c; border-bottom: 2px solid #e74c3c; padding-bottom: 5px;">🚨 Failed Login Protection</h2>

            <div style="margin: 20px 0;">
                <p><strong>Maximum 5 password attempts allowed</strong></p>
                <p>After 5 failed attempts, you get two options:</p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li><strong>Exit Application</strong></li>
                    <li><strong>Factory Reset</strong> (clears all data and security settings)</li>
                </ul>
            </div>

            <h2 style="color: #9b59b6; border-bottom: 2px solid #9b59b6; padding-bottom: 5px;">🔄 Factory Reset</h2>

            <div style="margin: 20px 0;">
                <p>If you forget your master password, factory reset will:</p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Remove all security settings</li>
                    <li>Clear the master password</li>
                    <li>Reset password protection to disabled</li>
                    <li>Allow you to start fresh</li>
                </ul>
            </div>

            <h2 style="color: #1abc9c; border-bottom: 2px solid #1abc9c; padding-bottom: 5px;">⚙️ Security Menu Options</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li><strong>"Require Password on Startup"</strong> - Toggle password protection on/off</li>
                    <li><strong>"Change Master Password"</strong> - Update your existing password</li>
                </ul>
            </div>

            <h2 style="color: #e67e22; border-bottom: 2px solid #e67e22; padding-bottom: 5px;">🔒 Protection States</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #d35400;">When Password Protection is Active:</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Jasmine prompts for password on every startup</li>
                    <li>All your websites, sessions, and data remain encrypted</li>
                    <li>No access to application features without correct password</li>
                </ul>

                <h3 style="color: #d35400;">When Password Protection is Disabled:</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Jasmine starts immediately without password prompt</li>
                    <li>All features accessible without authentication</li>
                    <li>Data remains saved but unprotected</li>
                </ul>
            </div>

            <h2 style="color: #16a085; border-bottom: 2px solid #16a085; padding-bottom: 5px;">💡 Best Practices</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Use a unique password not used elsewhere</li>
                    <li>Include numbers, letters, and special characters</li>
                    <li>Avoid easily guessable information</li>
                    <li>Keep a secure backup of your password</li>
                    <li>Enable password protection if you store sensitive login information</li>
                </ul>
            </div>

            <h2 style="color: #8e44ad; border-bottom: 2px solid #8e44ad; padding-bottom: 5px;">🔐 Security Implementation</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>SHA-256 encryption with custom salt</li>
                    <li>No plain text password storage</li>
                    <li>Secure settings storage</li>
                    <li>Memory-safe password handling</li>
                </ul>
            </div>

            <div style="background-color: #d1ecf1; padding: 15px; border-radius: 8px; margin: 25px 0;">
                <h3 style="color: #0c5460; margin-top: 0;">🛡️ Complete Protection</h3>
                <p style="margin-bottom: 0;">This security system ensures your browsing profiles, saved websites, sessions, and any stored login references remain protected even if someone gains access to your computer.</p>
            </div>
        </div>
    )";
}

QString HelpMenuDialog::getTwoFAContent()
{
    return R"(
        <div style="font-family: Arial, sans-serif; line-height: 1.6; color: #2c3e50;">
            <h1 style="color: #e74c3c; text-align: center; margin-bottom: 20px;">🔐 2FA Code Generator</h1>

            <div style="background-color: #ecf0f1; padding: 15px; border-radius: 8px; margin-bottom: 25px;">
                <h2 style="color: #2c3e50; margin-top: 0;">Two-Factor Authentication Manager</h2>
                <p>Jasmine includes a built-in Two-Factor Authentication (2FA) code generator that helps you manage and generate time-based one-time passwords (TOTP) for your accounts.</p>
            </div>

            <h2 style="color: #3498db; border-bottom: 2px solid #3498db; padding-bottom: 5px;">❓ What is 2FA?</h2>

            <div style="margin: 20px 0;">
                <p>Two-Factor Authentication adds an extra layer of security to your accounts by requiring a second form of verification beyond just your password. This usually involves a 6-digit code that changes every 30 seconds.</p>
            </div>

            <h2 style="color: #27ae60; border-bottom: 2px solid #27ae60; padding-bottom: 5px;">🚀 Key Features</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Generate 6-digit TOTP codes for any 2FA-enabled account</li>
                    <li>Real-time code updates every 30 seconds</li>
                    <li>Visual countdown timer showing when codes refresh</li>
                    <li>One-click code copying to clipboard</li>
                    <li>Secure local storage of account secrets</li>
                    <li>Support for multiple accounts from different services</li>
                </ul>
            </div>

            <h2 style="color: #f39c12; border-bottom: 2px solid #f39c12; padding-bottom: 5px;">📱 How to Access</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Open the 2FA Manager from the Toolbar icon or the Tools Menu</li>
                    <li>The manager opens in a separate window</li>
                    <li>Resizable interface with accounts list and code display</li>
                </ul>
            </div>

            <h2 style="color: #9b59b6; border-bottom: 2px solid #9b59b6; padding-bottom: 5px;">➕ Adding 2FA Accounts</h2>

            <div style="margin: 20px 0;">
                <ol style="margin: 10px 0; padding-left: 25px;">
                    <li>Click "Add Account" button</li>
                    <li>Enter account name (e.g., "GitHub", "Google", "Discord")</li>
                    <li>Paste the secret key from the website's 2FA setup</li>
                    <li>Optionally enter the issuer/company name</li>
                    <li>Click OK to save</li>
                </ol>
            </div>

            <div style="background-color: #fff3cd; padding: 15px; border-radius: 8px; margin: 15px 0;">
                <h3 style="color: #856404; margin-top: 0;">🔍 Where to Find Secret Keys</h3>
                <p>When enabling 2FA on websites, they typically show:</p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>A QR code for mobile apps</li>
                    <li>A text secret key (what you need for Jasmine)</li>
                    <li>Look for "Can't scan QR code?" or "Manual entry" options</li>
                </ul>
            </div>

            <h2 style="color: #e74c3c; border-bottom: 2px solid #e74c3c; padding-bottom: 5px;">🔢 Using Generated Codes</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Select an account from the list</li>
                    <li>Current 6-digit code displays in large text</li>
                    <li>Countdown timer shows seconds until next refresh</li>
                    <li>Click "Copy Code to Clipboard" for easy pasting</li>
                    <li>Codes automatically update every 30 seconds</li>
                </ul>
            </div>

            <h2 style="color: #1abc9c; border-bottom: 2px solid #1abc9c; padding-bottom: 5px;">👁️ Visual Indicators</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Progress bar shows time remaining (green → yellow → red)</li>
                    <li>Large, easy-to-read monospace font for codes</li>
                    <li>Clear countdown text showing refresh time</li>
                    <li>Copy button changes to "Copied!" for confirmation</li>
                </ul>
            </div>

            <h2 style="color: #e67e22; border-bottom: 2px solid #e67e22; padding-bottom: 5px;">⚙️ Account Management</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>View all your 2FA accounts in organized list</li>
                    <li>Delete accounts you no longer need</li>
                    <li>Accounts persist between application restarts</li>
                    <li>Secure local storage (not cloud-synced)</li>
                </ul>
            </div>

            <h2 style="color: #8e44ad; border-bottom: 2px solid #8e44ad; padding-bottom: 5px;">🔒 Security Notes</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Secret keys are stored locally on your device</li>
                    <li>No internet connection required for code generation</li>
                    <li>Codes are generated using industry-standard TOTP algorithm</li>
                    <li>Same codes as Google Authenticator, Authy, etc.</li>
                </ul>
            </div>

            <h2 style="color: #16a085; border-bottom: 2px solid #16a085; padding-bottom: 5px;">🌐 Supported Services</h2>

            <div style="margin: 20px 0;">
                <p>Works with any service that supports TOTP 2FA:</p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Google/Gmail accounts</li>
                    <li>GitHub</li>
                    <li>Discord</li>
                    <li>Microsoft accounts</li>
                    <li>Banking websites</li>
                    <li>Social media platforms</li>
                    <li>And many more</li>
                </ul>
            </div>

            <h2 style="color: #d35400; border-bottom: 2px solid #d35400; padding-bottom: 5px;">📋 Workflow Example</h2>

            <div style="margin: 20px 0;">
                <ol style="margin: 10px 0; padding-left: 25px;">
                    <li>Enable 2FA on GitHub</li>
                    <li>Copy the secret key from GitHub's setup page</li>
                    <li>Add account in Jasmine's 2FA Manager</li>
                    <li>When logging into GitHub, select the account</li>
                    <li>Copy the current 6-digit code</li>
                    <li>Paste into GitHub's 2FA prompt</li>
                </ol>
            </div>

            <h2 style="color: #7b1fa2; border-bottom: 2px solid #7b1fa2; padding-bottom: 5px;">💡 Benefits Over Mobile Apps</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Access codes directly on your computer</li>
                    <li>No need to grab your phone</li>
                    <li>Larger, easier-to-read display</li>
                    <li>Integrated with your browsing workflow</li>
                    <li>Quick clipboard copying</li>
                </ul>
            </div>

            <h2 style="color: #388e3c; border-bottom: 2px solid #388e3c; padding-bottom: 5px;">⏰ Time Synchronization</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Codes are time-based (30-second intervals)</li>
                    <li>Uses your system clock for accuracy</li>
                    <li>Same timing as other authenticator apps</li>
                    <li>Automatic refresh every second</li>
                </ul>
            </div>

            <div style="background-color: #d1ecf1; padding: 15px; border-radius: 8px; margin: 25px 0;">
                <h3 style="color: #0c5460; margin-top: 0;">🌟 Integrated Security</h3>
                <p style="margin-bottom: 0;">This 2FA manager eliminates the need for separate authenticator apps while providing the same security benefits, making it convenient to access your two-factor codes directly within Jasmine.</p>
            </div>
        </div>
    )";
}

QString HelpMenuDialog::getDataManagementContent()
{
    return R"(
        <div style="font-family: Arial, sans-serif; line-height: 1.6; color: #2c3e50;">
            <h1 style="color: #e74c3c; text-align: center; margin-bottom: 20px;">🗂️ Data Management & Privacy</h1>

            <div style="background-color: #ecf0f1; padding: 15px; border-radius: 8px; margin-bottom: 25px;">
                <h2 style="color: #2c3e50; margin-top: 0;">Session & Profile Data Management</h2>
                <p>Jasmine provides comprehensive tools to manage your browsing data, sessions, and privacy settings. Control what data is stored and when to clear it.</p>
            </div>

            <h2 style="color: #3498db; border-bottom: 2px solid #3498db; padding-bottom: 5px;">🧹 Clean Current Session Data</h2>

            <div style="margin: 20px 0;">
                <p><strong>What it does:</strong></p>
                <p>Clears browsing data from all currently active sessions and the shared profile.</p>

                <p><strong>Data removed:</strong></p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>All cookies from active sessions</li>
                    <li>HTTP cache from all profiles</li>
                    <li>Visited links history</li>
                    <li>Temporary browsing data</li>
                </ul>

                <p><strong>When to use:</strong></p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>After browsing sensitive websites</li>
                    <li>When sharing your computer</li>
                    <li>To free up storage space</li>
                    <li>For privacy after online shopping/banking</li>
                </ul>
            </div>

            <h2 style="color: #27ae60; border-bottom: 2px solid #27ae60; padding-bottom: 5px;">👥 Clean Shared Profile Data</h2>

            <div style="margin: 20px 0;">
                <p><strong>What it does:</strong></p>
                <p>Clears browsing data only from the shared profile, leaving separate tab profiles untouched.</p>

                <p><strong>Data removed:</strong></p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Shared profile cookies only</li>
                    <li>Shared profile cache</li>
                    <li>Shared profile visited links</li>
                </ul>

                <p><strong>What's preserved:</strong></p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Individual tab profile data</li>
                    <li>Private profile sessions</li>
                    <li>Separate profile cookies and cache</li>
                </ul>

                <p><strong>When to use:</strong></p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>When you want to keep private profiles intact</li>
                    <li>To clear general browsing without affecting work profiles</li>
                    <li>Selective privacy cleaning</li>
                </ul>
            </div>

            <h2 style="color: #e74c3c; border-bottom: 2px solid #e74c3c; padding-bottom: 5px;">🏭 Restore Factory Defaults</h2>

            <div style="margin: 20px 0;">
                <p><strong>What it does:</strong></p>
                <p>Completely resets Jasmine to its original state, removing all user data and settings.</p>

                <p><strong>Data removed:</strong></p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>All saved websites and bookmarks</li>
                    <li>All saved sessions</li>
                    <li>All application settings and preferences</li>
                    <li>Security settings and master passwords</li>
                    <li>All browsing data (cookies, cache, history)</li>
                    <li>Application data directories</li>
                    <li>Profile configurations</li>
                </ul>
            </div>

            <div style="background-color: #f8d7da; padding: 15px; border-radius: 8px; margin: 15px 0; border-left: 4px solid #dc3545;">
                <h3 style="color: #721c24; margin-top: 0;">⚠️ Factory Reset Warning</h3>
                <p style="margin-bottom: 0;"><strong>This action cannot be undone!</strong></p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>All your saved data will be permanently lost</li>
                    <li>Application will close automatically after reset</li>
                    <li>You'll need to restart Jasmine manually</li>
                    <li>All customizations will be lost</li>
                </ul>
            </div>

            <h2 style="color: #f39c12; border-bottom: 2px solid #f39c12; padding-bottom: 5px;">📍 How to Access These Features</h2>

            <div style="margin: 20px 0;">
                <p>All data management options are located in the <strong>Sessions</strong> menu:</p>
                <ol style="margin: 10px 0; padding-left: 25px;">
                    <li>Click on "Sessions" in the menu bar</li>
                    <li>Scroll to the bottom section</li>
                    <li>Choose your desired cleaning option</li>
                    <li>Confirm the action in the dialog box</li>
                </ol>
            </div>

            <h2 style="color: #9b59b6; border-bottom: 2px solid #9b59b6; padding-bottom: 5px;">🔄 Data Types Explained</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #8e44ad;">Cookies</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Login sessions and preferences</li>
                    <li>Shopping cart contents</li>
                    <li>Website customizations</li>
                </ul>

                <h3 style="color: #8e44ad;">HTTP Cache</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Temporarily stored website files</li>
                    <li>Images, scripts, and stylesheets</li>
                    <li>Speeds up repeat visits</li>
                </ul>

                <h3 style="color: #8e44ad;">Visited Links</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>History of visited websites</li>
                    <li>Link color changes (visited vs unvisited)</li>
                    <li>Navigation history</li>
                </ul>
            </div>

            <h2 style="color: #1abc9c; border-bottom: 2px solid #1abc9c; padding-bottom: 5px;">🛡️ Privacy Recommendations</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #16a085;">Regular Cleaning (Weekly)</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Use "Clean Shared Profile Data" for routine maintenance</li>
                    <li>Keeps private profiles intact</li>
                    <li>Maintains good performance</li>
                </ul>

                <h3 style="color: #16a085;">Deep Cleaning (Monthly)</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Use "Clean Current Session Data" for thorough cleanup</li>
                    <li>Clears all active session data</li>
                    <li>Good for privacy and storage</li>
                </ul>

                <h3 style="color: #16a085;">Emergency Cleaning</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>After using public computers</li>
                    <li>When selling or giving away device</li>
                    <li>After security concerns</li>
                </ul>
            </div>

            <h2 style="color: #e67e22; border-bottom: 2px solid #e67e22; padding-bottom: 5px;">💡 Smart Usage Tips</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li><strong>Before cleaning:</strong> Save any important sessions you want to keep</li>
                    <li><strong>Profile separation:</strong> Use private profiles for sensitive browsing</li>
                    <li><strong>Regular maintenance:</strong> Clean shared profile weekly, all data monthly</li>
                    <li><strong>Factory reset:</strong> Only use when starting completely fresh</li>
                    <li><strong>Backup important data:</strong> Export sessions before major cleaning</li>
                </ul>
            </div>

            <h2 style="color: #8e44ad; border-bottom: 2px solid #8e44ad; padding-bottom: 5px;">⚡ Performance Benefits</h2>

            <div style="margin: 20px 0;">
                <p><strong>Regular data cleaning provides:</strong></p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Faster application startup</li>
                    <li>Reduced memory usage</li>
                    <li>More available storage space</li>
                    <li>Improved browsing performance</li>
                    <li>Better privacy protection</li>
                </ul>
            </div>

            <div style="background-color: #d1ecf1; padding: 15px; border-radius: 8px; margin: 25px 0;">
                <h3 style="color: #0c5460; margin-top: 0;">🎯 Choose the Right Tool</h3>
                <p style="margin-bottom: 0;">
                    <strong>Shared Profile Clean:</strong> For routine maintenance<br>
                    <strong>Current Session Clean:</strong> For thorough privacy cleaning<br>
                    <strong>Factory Reset:</strong> For complete fresh start
                </p>
            </div>
        </div>
    )";
}

QString HelpMenuDialog::getDownloadManagerContent()
{
    return R"(
        <div style="font-family: Arial, sans-serif; line-height: 1.6; color: #2c3e50;">
            <h1 style="color: #e74c3c; text-align: center; margin-bottom: 20px;">📥 Download Manager</h1>

            <div style="background-color: #ecf0f1; padding: 15px; border-radius: 8px; margin-bottom: 25px;">
                <h2 style="color: #2c3e50; margin-top: 0;">Integrated Download Management</h2>
                <p>Jasmine includes a comprehensive download manager that handles all your file downloads with progress tracking, organization, and easy access to downloaded files.</p>
            </div>

            <h2 style="color: #3498db; border-bottom: 2px solid #3498db; padding-bottom: 5px;">🚀 Key Features</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Real-time download progress tracking</li>
                    <li>Download speed and time remaining calculations</li>
                    <li>Automatic file organization in dedicated folder</li>
                    <li>Duplicate filename handling</li>
                    <li>One-click access to files and folders</li>
                    <li>Download history management</li>
                    <li>Cancel active downloads</li>
                    <li>Clean interface with visual progress bars</li>
                </ul>
            </div>

            <h2 style="color: #27ae60; border-bottom: 2px solid #27ae60; padding-bottom: 5px;">📍 How to Access Downloads</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #229954;">Opening the Download Manager</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Click the <strong>Downloads</strong> icon in the toolbar</li>
                    <li>Or go to <strong>View → Downloads</strong> in the menu</li>
                    <li>Download window opens showing all current and past downloads</li>
                </ul>

                <h3 style="color: #229954;">Download Location</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Files are saved to: <code>Downloads/Jasmine/</code></li>
                    <li>Organized in your system's default Downloads folder</li>
                    <li>Automatic folder creation if it doesn't exist</li>
                </ul>
            </div>

            <h2 style="color: #f39c12; border-bottom: 2px solid #f39c12; padding-bottom: 5px;">📊 Download Progress Tracking</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #d68910;">Real-time Information</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li><strong>File name and size:</strong> Clear identification of what's downloading</li>
                    <li><strong>Progress bar:</strong> Visual representation of download completion</li>
                    <li><strong>Speed indicator:</strong> Current download speed (KB/s, MB/s)</li>
                    <li><strong>Time remaining:</strong> Estimated completion time</li>
                    <li><strong>Status updates:</strong> Starting, downloading, completed, cancelled</li>
                </ul>

                <h3 style="color: #d68910;">Progress Display</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Percentage completion with visual progress bar</li>
                    <li>Downloaded size vs. total file size</li>
                    <li>Dynamic speed calculations updated every second</li>
                    <li>Color-coded status indicators</li>
                </ul>
            </div>

            <h2 style="color: #9b59b6; border-bottom: 2px solid #9b59b6; padding-bottom: 5px;">🎛️ Download Controls</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #8e44ad;">During Download</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li><strong>Cancel Button:</strong> Stop active downloads immediately</li>
                    <li><strong>Open Folder:</strong> Access download directory anytime</li>
                    <li><strong>Progress Monitoring:</strong> Watch real-time progress</li>
                </ul>

                <h3 style="color: #8e44ad;">After Download</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li><strong>Open File:</strong> Launch downloaded file directly</li>
                    <li><strong>Open Folder:</strong> Navigate to file location</li>
                    <li><strong>Remove from List:</strong> Clean up download history</li>
                </ul>
            </div>

            <h2 style="color: #1abc9c; border-bottom: 2px solid #1abc9c; padding-bottom: 5px;">🗂️ File Organization</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #16a085;">Automatic Organization</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>All downloads saved to dedicated Jasmine folder</li>
                    <li>Automatic duplicate filename handling</li>
                    <li>Files renamed with numbers: <code>file.pdf</code>, <code>file (1).pdf</code>, <code>file (2).pdf</code></li>
                    <li>Preserves original file extensions</li>
                </ul>

                <h3 style="color: #16a085;">Smart Naming</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Uses original filename from website</li>
                    <li>Fallback to "download" if no name available</li>
                    <li>Prevents file overwrites automatically</li>
                </ul>
            </div>

            <h2 style="color: #e67e22; border-bottom: 2px solid #e67e22; padding-bottom: 5px;">🧹 Download Management</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #d35400;">Window Controls</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li><strong>Clear Finished:</strong> Remove completed/cancelled downloads from list</li>
                    <li><strong>Open Downloads Folder:</strong> Quick access to download directory</li>
                    <li><strong>Individual Remove:</strong> Remove specific items from history</li>
                </ul>

                <h3 style="color: #d35400;">List Management</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Chronological list of all downloads</li>
                    <li>Persistent across application restarts</li>
                    <li>Easy cleanup of old downloads</li>
                    <li>Empty state message when no downloads</li>
                </ul>
            </div>

            <h2 style="color: #8e44ad; border-bottom: 2px solid #8e44ad; padding-bottom: 5px;">💡 Download States</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #7b1fa2;">Active Downloads</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li><strong>Starting:</strong> Download initializing</li>
                    <li><strong>In Progress:</strong> Actively downloading with progress</li>
                    <li><strong>Speed Display:</strong> Real-time transfer rate</li>
                    <li><strong>Cancel Option:</strong> Stop button available</li>
                </ul>

                <h3 style="color: #7b1fa2;">Completed Downloads</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li><strong>Completed:</strong> Successfully downloaded</li>
                    <li><strong>Cancelled:</strong> User stopped download</li>
                    <li><strong>Interrupted:</strong> Network or system error</li>
                    <li><strong>File Access:</strong> Open file/folder buttons available</li>
                </ul>
            </div>

            <h2 style="color: #d32f2f; border-bottom: 2px solid #d32f2f; padding-bottom: 5px;">⚡ Performance Features</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li><strong>Efficient Memory Usage:</strong> Minimal resource consumption</li>
                    <li><strong>Background Downloads:</strong> Continue while browsing</li>
                    <li><strong>Multiple Downloads:</strong> Handle several files simultaneously</li>
                    <li><strong>Speed Calculation:</strong> Accurate transfer rate monitoring</li>
                    <li><strong>Progress Updates:</strong> Smooth, real-time progress tracking</li>
                </ul>
            </div>

            <h2 style="color: #388e3c; border-bottom: 2px solid #388e3c; padding-bottom: 5px;">🔧 Technical Details</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #2e7d32;">File Size Formatting</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Automatic unit conversion: B → KB → MB → GB</li>
                    <li>Decimal precision for readability</li>
                    <li>Speed shown as size per second</li>
                </ul>

                <h3 style="color: #2e7d32;">Time Calculations</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Remaining time based on current speed</li>
                    <li>Format: seconds, minutes, hours as appropriate</li>
                    <li>Dynamic updates as speed changes</li>
                </ul>
            </div>

            <h2 style="color: #5d4037; border-bottom: 2px solid #5d4037; padding-bottom: 5px;">🎯 Usage Tips</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li><strong>Monitor Progress:</strong> Keep download window open to watch progress</li>
                    <li><strong>Multiple Downloads:</strong> Start several downloads simultaneously</li>
                    <li><strong>Quick Access:</strong> Use "Open Folder" for easy file management</li>
                    <li><strong>Clean History:</strong> Regularly clear finished downloads</li>
                    <li><strong>Cancel if Needed:</strong> Stop unwanted downloads immediately</li>
                    <li><strong>File Organization:</strong> Downloads are automatically organized</li>
                </ul>
            </div>

            <div style="background-color: #fff3cd; padding: 15px; border-radius: 8px; margin: 15px 0;">
                <h3 style="color: #856404; margin-top: 0;">📁 Platform-Specific Notes</h3>
                <p><strong>Flathub Version:</strong> Shows download location in dialog</p>
                <p><strong>Standard Version:</strong> Opens file manager directly</p>
                <p style="margin-bottom: 0;"><strong>All Platforms:</strong> Downloads saved to system Downloads folder under "Jasmine" subdirectory</p>
            </div>

            <div style="background-color: #d1ecf1; padding: 15px; border-radius: 8px; margin: 25px 0;">
                <h3 style="color: #0c5460; margin-top: 0;">🌟 Integrated Experience</h3>
                <p style="margin-bottom: 0;">The download manager seamlessly integrates with your browsing experience, automatically handling all file downloads while providing full control and visibility over the download process.</p>
            </div>
        </div>
    )";
}

QString HelpMenuDialog::getSitesSessionsContent()
{
    return QString(
        "<h3>Managing Sites</h3>"
        "<p><strong>Creating a New Site:</strong></p>"
        "<ul>"
        "<li>Press the <strong>Clear</strong> button to clear existing fields</li>"
        "<li>Fill in the <strong>URL</strong> and <strong>Title</strong> (required fields)</li>"
        "<li>Optionally add <strong>Username</strong>, <strong>Password</strong>, and <strong>Comments</strong></li>"
        "<li>Press the <strong>Add</strong> button to save the site</li>"
        "</ul>"

        "<h3>Managing Sessions</h3>"
        "<p><strong>Creating a New Session:</strong></p>"
        "<ul>"
        "<li>Ensure at least one tab is open in the webview</li>"
        "<li>Select <strong>Save Current Session</strong> from the menu or toolbar</li>"
        "<li>Give your session a name and click <strong>OK</strong></li>"
        "<li>Sessions are automatically assigned a randomly generated SVG icon</li>"
        "</ul>"

        "<h3>Editing Sites and Sessions</h3>"
        "<p><strong>To edit any site or session:</strong></p>"
        "<ul>"
        "<li>Select the item you want to modify</li>"
        "<li>Enter the new values in the appropriate fields</li>"
        "<li>Press the <strong>Update</strong> button to save changes</li>"
        "<li>For sessions: Click the small <strong>Edit</strong> button next to the icon to change it, then click <strong>Update</strong></li>"
        "</ul>"
        );
}

QString HelpMenuDialog::getOnSecurityContent()
{
    return QString(
        "<h3>Security Features Overview</h3>"
        "<p>Jasmine provides several optional security features designed for your convenience. "
        "You are completely free to use or not use any of these features based on your preferences.</p>"

        "<h4>Username & Password Storage</h4>"
        "<ul>"
        "<li>Storing credentials in website entries is <strong>completely optional</strong></li>"
        "<li>Leave these fields blank if you prefer to use your own credentials manager</li>"
        "<li>Stored credentials are saved locally on your device only in binary format</li>"
        "<li>No data is transmitted over the network</li>"
        "</ul>"

        "<h4>Master Password Protection</h4>"
        "<ul>"
        "<li>Optional feature to protect access to Jasmine</li>"
        "<li>When enabled, you'll need to enter your master password on startup</li>"
        "<li>Choose a strong, memorable password and store it safely</li>"
        "<li>If forgotten, you'll need to perform a factory reset</li>"
        "</ul>"

        "<h4>Two-Factor Authentication (2FA) Manager</h4>"
        "<ul>"
        "<li>Optional convenience tool for generating TOTP codes</li>"
        "<li>Helps manage 2FA codes for your various accounts</li>"
        "<li>All secrets are stored locally on your device in binary format</li>"
        "<li>Use only if you're comfortable with local storage</li>"
        "</ul>"

        "<h4>Security Disclaimer</h4>"
        "<p><em>While every reasonable effort has been made to implement a secure framework "
        "and all sensitive info is stored in binary format within Jasmine, "
        "these features are provided as conveniences rather than guarantees. Users are responsible "
        "for deciding what information to store based on their individual security requirements and risk tolerance.</em></p>"

        "<p><strong>Recommendation:</strong> For maximum security, consider using dedicated password managers "
        "and letting your browser handle credential storage.</p>"
        );
}

QString HelpMenuDialog::getOnNewStorageSystemContent() {
    return QString(
        "<h3>New Storage System (Jasmine 1.1.0 and onwards)</h3>"
        "<p>This version of Jasmine uses a new storage system that improves performance and efficiency by using symlinks for profile data. This means that instead of copying entire profile directories, only links are created, saving disk space and speeding up operations.</p>"
        "<h4>Recommendations:</h4>"
        "<p>To fully leverage the new storage system and ensure optimal performance, we highly recommend performing a factory reset. This will clear all old data and provide a clean start. You can find the factory reset option in the 'Sessions' menu.</p>"
        "<h4>Clean up orphaned profile directories on startup:</h4>"
        "<p>As part of the new storage system, a checkbox is available in the 'Tools' menu to automatically clean up unused, orphaned profile directories at application startup. It is STRONGLY RECOMMENDED to keep this option enabled UNLESS you also have old-format sessions saved.</p>"
        "<h4>Handling Old-Format Sessions:</h4>"
        "<p>If you have existing sessions that use the old directory-based storage, you can continue to use them. However, we strongly advise against adding new tabs or saving changes to these older sessions. To ensure optimal performance and avoid potential issues, it's best to create new sessions using the new storage system.</p>"
        );
}




