# 🌸 Jasmine

![Jasmine Screenshot](screenshots/Jasmine-Main.png)

**Website & Session Manager**

A comprehensive web launcher and session management application that transforms your scattered bookmarks and browser tabs into an organized, launchable workspace. Jasmine also includes built-in Internet Radio, IPTV, and Podcast Manager for a complete media and productivity suite.

**Download**

| Platform | Where to Get It |
| :--- | :--- |
| **Linux** | [Flathub](https://flathub.org/en/apps/search?q=alamahant) |
| **Windows and Mac** | [Buy on Gumroad](https://jnanadhakini.gumroad.com/) - Pre-compiled binary, no compilation needed |

---

## 📋 Table of Contents
- [Overview](#overview)
- [Key Features](#key-features)
- [Profile System](#profile-system)
- [Real-World Use Cases](#real-world-use-cases)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [Advanced Usage](#advanced-usage)
- [Security](#security)
- [Contributing](#contributing)
- [License](#license)

## 🚀 Overview
**Version:** 1.0.0  
**Built with:** Qt Framework  
**Platform:** Cross-platform  

Jasmine combines bookmarking, multi-tab session handling, flexible browsing profiles, Internet Radio, IPTV, Podcast management, and integrated web utilities into one streamlined tool. Perfect for productivity enthusiasts, multi-account managers, and media lovers.

## ✨ Key Features

### 🔥 Core Functionality
- **📚 Smart Bookmarking** - Store websites with titles, URLs, comments, favicons, and login references
- **💾 Session Management** - Create, save, and restore multi-tab browsing sessions with one click
- **🔒 Flexible Web Profiles** - Choose between shared, named-shared (user-created), or private (incognito) profiles per tab
- **👥 Multi-Account Support** - Simultaneously access multiple accounts on the same service without conflicts
- **⚡ Launch Control** - Quick-launch individual websites or entire browsing sessions instantly

### 🎵 Media & Entertainment
- **📻 Internet Radio** - Browse, search, and play thousands of internet radio stations with local icon caching
- **📺 IPTV** - Import M3U playlists, browse channels by category, and watch live TV streams
- **🎙️ Podcast Manager** - Subscribe to podcasts via iTunes search or RSS feed URLs, manage episodes, and play audio

### 🛠️ Integrated Utilities
- **📥 Download Manager** - Complete download management system with progress tracking
- **📸 Screenshot Capture** - Take and save screenshots of web pages for documentation
- **🔑 Login Reference** - Store username and password reminders with privacy controls
- **🔐 2FA Integration** - Built-in two-factor authentication code generator
- **📊 Visit Tracking** - Monitor site usage with visit counts and timestamps

### 🔐 Security Features
- **🛡️ Master Password Protection** - Secure your data with encrypted password protection
- **🔒 Private Profile System** - Completely isolated browsing environments
- **🕵️ Incognito Mode** - Temporary private sessions without saving data

## 🎯 Perfect For
- **Managing multiple accounts** on the same service with isolated, named-shared, or shared sessions
- **Grouping entire workflows** under a single named profile - multiple different sites sharing the same session data
- **Users who work with multiple web applications** daily and need integrated utilities
- **Developers managing various development and staging environments** with documentation needs
- **Anyone who wants organized access** to their frequently-used sites with full session control
- **Teams needing quick access** to shared web resources with built-in productivity tools
- **Media enthusiasts** who want Internet Radio, IPTV, and podcasts alongside their browsing

## 🔒 Jasmine's Profile System - Complete Guide

### What Are Profiles?
Profiles in Jasmine are browsing environments that determine how cookies, login sessions, and data are shared or isolated. Jasmine offers three distinct profile types:

- **Shared Profile (Default):** One common profile shared across all tabs. Cookies and data persist normally. Perfect for everyday browsing.

- **Named-Shared Profiles (User-Created):** Create a named profile (e.g., "Work", "Personal", "Client X") and launch multiple tabs under that same name. All tabs under that named profile share cookies and session data with each other - regardless of whether they are the same website or different websites. For example, under a "Work" named profile, you can have Gmail, Slack, Jira, and GitHub all sharing login sessions seamlessly. Different named profiles remain completely isolated from each other and from the default shared profile.

- **Private Profiles (Incognito):** Completely isolated environment with no local persistence — no cookies, history, or cache are saved after a tab closes. However, unlike typical incognito modes, Jasmine allows you to save private profile tabs as part of a session and relaunch them later, preserving their temporary isolation. Perfect for one-off logins, testing, sensitive browsing, or repeated use where you always want a clean slate.

### Profile Comparison

| Feature | Shared | Named-Shared | Private |
| :--- | :--- | :--- | :--- |
| Persists after restart | Yes | Yes | Optional (can save in sessions) |
| Shares data across tabs | Yes (all tabs) | Yes (only tabs under the same profile name) | No |
| Can share data across different websites | Yes | Yes | No |
| Isolated from other profiles | No | Yes | Yes |
| Can save in sessions | Yes | Yes | Yes |
| Best for | Everyday browsing | Grouped workflows (multiple sites working together) | Temporary access or clean-slate repeated use |

## 🌟 Real-World Use Cases with Examples

### 💼 Work Session with Multiple Sites
**Scenario:** You need Gmail, Slack, Jira, GitHub, and Google Drive all logged into your work accounts.

**Setup with Named-Shared:**
1. Create a named-shared profile called "Work"
2. Launch Gmail under "Work" → Log into work email
3. Launch Slack under "Work" → Log into work Slack
4. Launch Jira under "Work" → Log into work Jira
5. Launch GitHub under "Work" → Log into work GitHub
6. Launch Google Drive under "Work" → Log into work Drive
7. Save all five tabs as "Full Work Session"

**Result:** One-click access to your entire work ecosystem. All sites share session data where needed (e.g., Google Drive recognizes your work Gmail), but nothing from this "Work" profile leaks into your personal browsing or other named profiles.

### 🏠 Personal Session with Multiple Sites
**Scenario:** You want your personal Gmail, YouTube, Google Photos, and Reddit all logged into your personal accounts.

**Setup:**
1. Create a named-shared profile called "Personal"
2. Launch Gmail under "Personal" → Personal email
3. Launch YouTube under "Personal" → Personal recommendations
4. Launch Google Photos under "Personal" → Personal photo library
5. Launch Reddit under "Personal" → Personal Reddit account
6. Save as "Personal Session"

**Result:** Your entire personal web life in one session. YouTube knows your Gmail account, Google Photos is already logged in, and nothing mixes with work.

### 📧 Gmail Multi-Account Management
**Scenario:** You have personal Gmail, work Gmail, and side-project Gmail accounts.

**Setup with Named-Shared:**
1. Create a named-shared profile called "Personal Email" → Launch Gmail, log into personal account
2. Create a named-shared profile called "Work Email" → Launch Gmail, log into work account
3. Create a named-shared profile called "Project Email" → Launch Gmail, log into side-project account
4. Save all three tabs as "All Gmail Accounts" session

**Result:** One-click access to all three Gmail accounts simultaneously. Each named profile is isolated from the others. No conflicts, no accidental cross-posting.

### 🛠️ Development Workflow
**Scenario:** You are working on Project A with its own set of tools, and Project B with a completely different set.

**Setup:**
1. Create named-shared profile "Project A"
   - GitHub (project A repos)
   - Jira (project A tickets)
   - AWS Console (project A infrastructure)
   - Sentry (project A error tracking)
2. Create named-shared profile "Project B"
   - GitLab (project B repos)
   - Trello (project B tasks)
   - Google Cloud Console (project B infrastructure)
   - Datadog (project B monitoring)
3. Save each as separate sessions

**Result:** Switch between entire project contexts instantly. Each project's tools share logins seamlessly, but Project A and Project B never mix.

### 📱 Social Media Management
**Scenario:** You manage personal accounts plus two business brands, each with multiple platforms.

**Setup:**
1. Named-shared profile "Personal"
   - Twitter (personal)
   - Instagram (personal)
   - Facebook (personal)
   - LinkedIn (personal)
2. Named-shared profile "Brand A"
   - Twitter (Brand A)
   - Instagram (Brand A)
   - Facebook (Brand A)
   - TikTok (Brand A)
3. Named-shared profile "Brand B"
   - Twitter (Brand B)
   - Instagram (Brand B)
   - Facebook (Brand B)
   - Pinterest (Brand B)
4. Save each as separate sessions

**Result:** Manage all accounts across all platforms. Each brand's social media tabs share data with each other but stay completely isolated from other brands.

### 🛒 E-commerce and Shopping
**Scenario:** You want separate shopping identities: personal buying, business purchasing, and selling.

**Setup:**
1. Named-shared profile "Personal Shopping"
   - Amazon (personal wishlist, payment methods)
   - eBay (personal buying account)
   - Etsy (personal account)
2. Named-shared profile "Business Purchasing"
   - Amazon Business (company purchases)
   - Staples (business supplies)
   - CDW (IT equipment)
3. Named-shared profile "Seller"
   - eBay seller account
   - PayPal Business
4. Save as separate sessions

**Result:** Each shopping identity keeps its own purchase history, saved items, and payment methods.

### 🎵 Media & Entertainment Setup
**Scenario:** You want to listen to Internet Radio, watch IPTV, and catch up on podcasts while browsing.

**Setup:**
1. Launch Internet Radio module → Browse and play your favorite stations
2. Launch IPTV module → Import an M3U playlist and watch live TV channels by category
3. Launch Podcast Manager → Subscribe to podcasts via iTunes search or RSS feed URL
4. Launch your regular browsing session in the shared profile
5. Save as "Entertainment + Browsing" session

**Result:** All your media and browsing in one window. Switch between radio, TV, podcasts, and websites without juggling separate apps.

## 🔄 Mix and Match Strategies

### Hybrid Sessions - Combining Profile Types
You can create sessions that mix shared, named-shared, and private profiles:

**Example "Full Work Day" Session:**
- **Gmail Work** (named-shared: "Work Email")
- **Google Calendar** (named-shared: "Work Email") → Shares session with Work Gmail
- **Slack** (named-shared: "Work Comms")
- **GitHub Work** (named-shared: "Work Dev")
- **Jira** (named-shared: "Work Dev") → Shares session with GitHub
- **Stack Overflow** (shared profile) → General research, no login needed
- **Company Intranet** (shared profile)
- **Personal Email Check** (private) → Quick one-off check without mixing data

**Result:** Different named profiles for different purposes (Email suite, Development tools), each sharing data internally, but isolated from each other. Shared profile for public sites. Private for temporary access.

### Testing and Development
- **Production site** (shared profile) → Normal user account
- **Staging site** (named-shared: "Staging Test") → Multiple staging tools sharing test data
- **Development site** (named-shared: "Dev Test") → Multiple dev tools sharing dev data
- **Admin panel** (private profile) → Administrative access with isolation
- **Analytics dashboard** (shared profile) → Can see data from production

### Privacy Levels
- **Banking and Financial** (private profile) → Maximum isolation for sensitive accounts
- **Work accounts** (named-shared per project or department)
- **Personal accounts** (named-shared: "Personal")
- **Family shared accounts** (named-shared: "Family") → Netflix, Spotify, shared calendar
- **General browsing** (shared profile) → Convenience for sites that can share data

## 🚀 Advanced Multi-Account Scenarios

### Freelancer or Consultant Setup
- **Client A project** (named-shared: "Client A")
  - Client A Gmail
  - Client A Slack
  - Client A GitHub
  - Client A AWS Console
- **Client B project** (named-shared: "Client B")
  - Client B Gmail
  - Client B Teams
  - Client B GitLab
  - Client B Cloud Console
- **My Business** (named-shared: "My Business")
  - Business email
  - Invoicing software
  - Accounting portal
- **General research** (shared profile)

### Content Creator Workflow
- **Gaming Channel** (named-shared: "Gaming")
  - YouTube (gaming channel)
  - Twitch dashboard
  - Discord (gaming community)
  - Social media (gaming accounts)
- **Educational Channel** (named-shared: "Education")
  - YouTube (edu channel)
  - Patreon
  - Email (edu contact)
  - Social media (edu accounts)
- **Research** (shared profile) → General content research

## 🔐 Two-Factor Authentication (2FA) Manager

Jasmine includes a built-in Two-Factor Authentication code generator that helps you manage and generate time-based one-time passwords (TOTP) for your accounts.

### What is 2FA?
Two-Factor Authentication adds an extra layer of security to your accounts by requiring a second form of verification beyond just your password. This usually involves a 6-digit code that changes every 30 seconds.

### Key Features
- Generate 6-digit TOTP codes for any 2FA-enabled account
- Real-time code updates every 30 seconds
- Visual countdown timer showing when codes refresh
- One-click code copying to clipboard
- Secure local storage of account secrets
- Support for multiple accounts from different services

### How to Access
- Open the 2FA Manager from the Toolbar icon or the Tools Menu
- The manager opens in a separate window
- Resizable interface with accounts list and code display

### Adding 2FA Accounts
1. Click "Add Account" button
2. Enter account name (e.g., "GitHub", "Google", "Discord")
3. Paste the secret key from the website's 2FA setup
4. Optionally enter the issuer or company name
5. Click OK to save

### Where to Find Secret Keys
When enabling 2FA on websites, they typically show:
- A QR code for mobile apps
- A text secret key (what you need for Jasmine)
- Look for "Can't scan QR code?" or "Manual entry" options

### Using Generated Codes
- Select an account from the list
- Current 6-digit code displays in large text
- Countdown timer shows seconds until next refresh
- Click "Copy Code to Clipboard" for easy pasting
- Codes automatically update every 30 seconds

### Supported Services
Works with any service that supports TOTP 2FA:
- Google and Gmail accounts
- GitHub
- Discord
- Microsoft accounts
- Banking websites
- Social media platforms
- And many more

## 🗂️ Data Management and Privacy

Jasmine provides comprehensive tools to manage your browsing data, sessions, and privacy settings.

### Clean Current Session Data
Clears browsing data from all currently active sessions and the shared profile.

**Data removed:**
- All cookies from active sessions
- HTTP cache from all profiles
- Visited links history
- Temporary browsing data

**When to use:**
- After browsing sensitive websites
- When sharing your computer
- To free up storage space
- For privacy after online shopping or banking

### Clean Shared Profile Data
Clears browsing data only from the shared profile, leaving separate tab profiles untouched.

**Data removed:**
- Shared profile cookies only
- Shared profile cache
- Shared profile visited links

**What is preserved:**
- Individual tab profile data
- Named-shared profile data
- Private profile sessions
- Separate profile cookies and cache

### Restore Factory Defaults
⚠️ **Warning: This action cannot be undone!**

Completely resets Jasmine to its original state, removing:
- All saved websites and bookmarks
- All saved sessions
- All application settings and preferences
- Security settings and master passwords
- All browsing data (cookies, cache, history)
- All named profiles and their data
- Application data directories
- Profile configurations

### How to Access
All data management options are located in the **Sessions** menu:
1. Click on "Sessions" in the menu bar
2. Scroll to the bottom section
3. Choose your desired cleaning option
4. Confirm the action in the dialog box

### Privacy Recommendations
- **Regular Cleaning (Weekly):** Use "Clean Shared Profile Data"
- **Deep Cleaning (Monthly):** Use "Clean Current Session Data"
- **Emergency Cleaning:** After using public computers or security concerns

## 📥 Download Manager

Jasmine includes a comprehensive download manager that handles all your file downloads with progress tracking, organization, and easy access.

### Key Features
- Real-time download progress tracking
- Download speed and time remaining calculations
- Automatic file organization in dedicated folder
- Duplicate filename handling
- One-click access to files and folders
- Download history management
- Cancel active downloads
- Clean interface with visual progress bars

### How to Access Downloads
**Opening the Download Manager:**
- Click the **Downloads** icon in the toolbar
- Or go to **View → Downloads** in the menu

**Download Location:**
- Files are saved to: `Downloads/Jasmine/`
- Organized in your system's default Downloads folder
- Automatic folder creation if it does not exist

### Download Progress Tracking
**Real-time Information:**
- File name and size: Clear identification of what is downloading
- Progress bar: Visual representation of download completion
- Speed indicator: Current download speed (KB/s, MB/s)
- Time remaining: Estimated completion time
- Status updates: Starting, downloading, completed, cancelled

### Download Controls
**During Download:**
- Cancel Button: Stop active downloads immediately
- Open Folder: Access download directory anytime
- Progress Monitoring: Watch real-time progress

**After Download:**
- Open File: Launch downloaded file directly
- Open Folder: Navigate to file location
- Remove from List: Clean up download history

### File Organization
**Automatic Organization:**
- All downloads saved to dedicated Jasmine folder
- Automatic duplicate filename handling
- Files renamed with numbers: `file.pdf`, `file (1).pdf`, `file (2).pdf`
- Preserves original file extensions

### Download Management
**Window Controls:**
- Clear Finished: Remove completed or cancelled downloads from list
- Open Downloads Folder: Quick access to download directory
- Individual Remove: Remove specific items from history

### Usage Tips
- Monitor Progress: Keep download window open to watch progress
- Multiple Downloads: Start several downloads simultaneously
- Quick Access: Use "Open Folder" for easy file management
- Clean History: Regularly clear finished downloads
- Cancel if Needed: Stop unwanted downloads immediately

## 📻 Internet Radio

Jasmine includes a full-featured Internet Radio player.

### Key Features
- Browse thousands of internet radio stations
- Search for stations by name, genre, or country
- Local icon caching for fast loading
- Play streams directly in the application
- Save favorite stations

### How to Use
1. Open the Internet Radio module from the toolbar or Tools menu
2. Browse or search for stations
3. Double-click a station to start playing
4. Enjoy live audio streaming

## 📺 IPTV Player

Watch live TV streams by importing M3U playlists.

### Key Features
- Import M3U and M3U8 playlists
- Browse channels by category
- Watch live TV streams
- Save and reload playlists

### How to Use
1. Open the IPTV module from the toolbar or Tools menu
2. Load an M3U playlist file
3. Browse channels by category
4. Select a channel to start watching

## 🎙️ Podcast Manager

Subscribe to and manage podcasts from multiple sources.

### Key Features
- Search for podcasts via iTunes API
- Subscribe using RSS feed URLs
- Manage episodes (download, play, mark as played)
- Built-in audio player
- Episode progress tracking

### How to Use
1. Open the Podcast Manager from the toolbar or Tools menu
2. Add podcasts by searching iTunes or entering an RSS feed URL
3. Browse episodes and click to play
4. Manage your subscription list and episode history

## 🔒 Security and Privacy Notice

**Optional Security Features:**
- Username and password storage in website entries is **completely optional**
- Master password protection is **optional** but recommended
- 2FA manager is an **optional convenience feature**

**Your Choice, Your Data:**
- Jasmine provides security features as conveniences, not guarantees
- You decide what information to store and what security features to enable
- For maximum security, rely on your browser's built-in password manager
- All data is stored locally on your device

**Disclaimer:**
While every effort has been made to implement reasonable security measures, users are responsible for deciding what information to store and which security features to enable based on their individual risk tolerance.

## 📦 Installation

### Prerequisites
- Qt Framework (LGPL v3)
- C++ compiler with C++17 support

### Building from Source
```bash
git clone [repository-url]
cd jasmine
qmake
make

## 🚀 Quick Start

### Step 1: Create Websites
1. Fill in the details panel (title, URL, comments)
2. Click **"Add Website"**
3. Website appears as a card in your library

### Step 2: Launch with Profile Control
- **Toggle OFF**: Shared profile (integrated browsing)
- **Toggle ON**: Private profile (completely isolated)

### Step 3: Save Sessions
1. Launch multiple websites with desired profile settings
2. Go to Sessions tab
3. Click **"Save Current Session"**
4. Name your session for future one-click restoration

## 🎯 Advanced Usage

### Session Management
- **Append** - Add session tabs to existing tabs
- **Replace** - Close existing tabs first, then launch session
- **Expand** - Launch existing session, add more tabs, save updated session

## 🔐 Security

### Master Password Protection
1. Go to **Security** → **"Require Password on Startup"**
2. Set your master password
3. Jasmine will require password on every startup

### Security Features
- SHA-256 encryption with salt
- 5 failed attempt protection
- Factory reset option for forgotten passwords
- Secure settings storage

⚠️ **Important:** If you forget your master password, you'll need to factory reset (clears all data).

## 💼 Use Cases

### 🏢 Business Manager
- Multiple client Gmail accounts (isolated)
- Different team Slack workspaces (isolated)
- Client-specific GitHub accounts (isolated)
- Shared documentation and research

### 🎬 Content Creator
- Multiple YouTube channels (isolated)
- Different social media accounts (isolated)
- Brand-specific profiles (isolated)
- Shared analytics and tools

### 💻 Developer
- Personal, work, and client GitHub accounts (isolated)
- Project-specific email accounts (isolated)
- Multiple cloud console accounts (isolated)
- Shared documentation and Stack Overflow

### 📱 Social Media Manager
- Multiple Twitter/Instagram/Facebook accounts (isolated)
- Client brand accounts (isolated)
- Campaign-specific profiles (isolated)

## 🎛️ Interface Controls
- **Dashboard/WebView Toggle** - Switch between management and browsing views
- **Dark/Light Theme Toggle** - Switch visual themes
- **Private Profile Toggle** - Control session isolation before launching

## 🎯 Key Benefits of This System

### 🔐 Security and Privacy
- No accidental cross-posting between accounts
- Complete data isolation prevents tracking across accounts
- Sensitive accounts remain completely separate from general browsing
- Incognito mode leaves no traces for temporary access

### ⚡ Productivity
- Instant access to all accounts without constant login/logout cycles
- Maintain context and state for each account/project
- Quick switching between different professional identities
- Organized workflow with purpose-built sessions

### 🔧 Flexibility
- Start in incognito mode, decide later whether to save
- Mix private and shared profiles in the same session
- Create specialized sessions for different life contexts
- Adapt profile strategy based on specific needs

## ⚠️ Important Notes
- **DO NOT LOG OUT** before saving sessions if you want to retain login status
- Private profiles are completely isolated - each has separate cookies, history, and data
- Sessions restore with original toggle states, but you can add new tabs with different settings

## 💪 The Power of Choice

Jasmine's private profile system gives you unprecedented control over your online identity management. You can be completely anonymous (incognito mode), maintain multiple persistent identities (saved private profiles), or mix approaches based on your specific needs. This level of flexibility is impossible with traditional browsers, making Jasmine a powerful tool for anyone managing multiple online accounts or requiring sophisticated privacy controls.

## 🤝 Contributing

We welcome contributions! Please read our contributing guidelines and submit pull requests for any improvements.

## 📄 License

This project is licensed under the GPL v3 License - see the [LICENSE](LICENSE) file for details.

## 🙏 Credits

**Framework:**
- Qt Framework (https://www.qt.io/) - Licensed under LGPL v3

**Icons:**
- Feather Icons (https://feathericons.com/) - Licensed under MIT License, Copyright (c) 2013-2017 Cole Bemis

---

**Copyright © 2025 Alamahant**  
Made with ❤️ for productivity enthusiasts and multi-account managers

*Transform your scattered bookmarks and browser tabs into an organized, launchable workspace with complete control over session isolation, account management, and integrated web utilities.*

