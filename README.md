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
- [Use Cases](#use-cases)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [Security](#security)
- [Contributing](#contributing)
- [License](#license)

## 🚀 Overview
**Version:** 1.0.0  
**Built with:** Qt Framework  
**Platform:** Cross-platform  

Jasmine combines bookmarking, multi-tab session handling, flexible browsing profiles, Internet Radio, IPTV, Podcast management, and integrated utilities into one streamlined tool.

## ✨ Key Features

### 🔥 Core
- **Smart Bookmarking** - Store websites with titles, URLs, comments, favicons, and login references
- **Session Management** - Create, save, and restore multi-tab sessions with one click
- **Flexible Profiles** - Choose between shared, named-shared (user-created), or private (incognito) profiles per tab
- **Multi-Account Support** - Access multiple accounts on the same service without conflicts

### 🎵 Media
- **Internet Radio** - Browse, search, and play thousands of stations with local icon caching
- **IPTV** - Import M3U playlists, browse channels by category, and watch live TV
- **Podcast Manager** - Subscribe via iTunes search or RSS, manage episodes, and play audio

### 🛠️ Utilities
- **Download Manager** - Progress tracking and file organization
- **Screenshot Capture** - Take and save web page screenshots
- **Login Reference** - Store username/password reminders with privacy controls
- **2FA Integration** - Built-in TOTP code generator
- **Visit Tracking** - Monitor site usage with counts and timestamps

### 🔐 Security
- **Master Password Protection** - Encrypted data protection
- **Private Profiles** - Completely isolated browsing environments

## 🔒 Profile System

Three distinct profile types:

- **Shared (Default):** One common profile across all tabs. Cookies persist normally. Best for everyday browsing.

- **Named-Shared (User-Created):** Create a profile name (e.g., "Work", "Personal"). All tabs under that name share cookies and session data - even across different websites. Under "Work", Gmail, Slack, Jira, and GitHub all stay logged in together. Different named profiles are completely isolated from each other.

- **Private (Incognito):** No local persistence after tab close, but can be saved in sessions and relaunched. Perfect for one-off logins, testing, or repeated clean-slate use.

| Feature | Shared | Named-Shared | Private |
| :--- | :--- | :--- | :--- |
| Persists after restart | Yes | Yes | Optional |
| Shares data across tabs | Yes (all) | Yes (same profile name) | No |
| Shares data across different sites | Yes | Yes | No |
| Isolated from other profiles | No | Yes | Yes |
| Can save in sessions | Yes | Yes | Yes |

## 💼 Use Cases

### Work Session
Create named profile "Work" → Launch Gmail, Slack, Jira, GitHub, Google Drive under it. All share logins. Save as session. One-click access to entire work ecosystem.

### Personal Session
Create named profile "Personal" → Launch Gmail, YouTube, Google Photos, Reddit. All share your personal accounts. Nothing mixes with work.

### Multiple Gmail Accounts
Create "Personal Email", "Work Email", "Project Email" → Each gets its own Gmail tab. All isolated from each other.

### Development Workflow
- "Project A" profile → GitHub, Jira, AWS Console, Sentry
- "Project B" profile → GitLab, Trello, GCP Console, Datadog
Switch between entire project contexts instantly.

### Social Media Management
- "Personal" profile → Twitter, Instagram, Facebook, LinkedIn
- "Brand A" profile → Twitter, Instagram, Facebook, TikTok
- "Brand B" profile → Twitter, Instagram, Facebook, Pinterest

### Freelancer Setup
- "Client A" → Gmail, Slack, GitHub, AWS
- "Client B" → Gmail, Teams, GitLab, Cloud Console
- "My Business" → Business email, invoicing, accounting
- Shared profile → General research

### Media & Browsing
Launch Internet Radio, IPTV, Podcast Manager alongside shared profile browsing. Save as session.

### Hybrid Session Example
- Gmail Work (named: "Work Email")
- Google Calendar (named: "Work Email") → shares with Gmail
- Slack (named: "Work Comms")
- GitHub (named: "Work Dev")
- Jira (named: "Work Dev") → shares with GitHub
- Stack Overflow (shared)
- Personal Email (private)

## 🔐 2FA Manager

Built-in TOTP code generator. Add accounts with secret keys from any 2FA-enabled service (Google, GitHub, Discord, banking, etc.). Codes auto-update every 30 seconds with copy-to-clipboard.

## 🗂️ Data Management

**Sessions menu options:**
- **Clean Current Session Data** - Cookies, cache, history from all active sessions
- **Clean Shared Profile Data** - Shared profile only, leaves named/private profiles intact
- **Restore Factory Defaults** - Complete reset (cannot be undone)

## 📥 Download Manager

Downloads save to `Downloads/Jasmine/`. Features real-time progress, speed, time remaining, cancel option, and one-click file/folder access.

## 📻 Internet Radio

Browse/search thousands of stations. Local icon caching. Double-click to play.

## 📺 IPTV Player

Import M3U/M3U8 playlists. Browse channels by category. Watch live streams.

## 🎙️ Podcast Manager

Subscribe via iTunes search or RSS URL. Manage episodes. Built-in audio player.

## 📦 Installation

### Prerequisites
- Qt Framework (LGPL v3)
- C++ compiler with C++17 support

### Building from Source

git clone [repository-url]
cd jasmine
qmake
make

---

## 🚀 Quick Start

1. **Add websites** - Fill details panel, click "Add Website"
2. **Choose profile** - Shared (default), named-shared, or private
3. **Launch** - Websites open as tabs with your chosen profile
4. **Save session** - Go to Sessions tab → "Save Current Session" → name it

### Session Management Options
- **Append** - Add session tabs to existing tabs
- **Replace** - Close existing tabs first, then launch session
- **Expand** - Launch session, add more tabs, save updated version

## 🔐 Security

### Master Password
1. Go to **Security** → **"Require Password on Startup"**
2. Set password (SHA-256 with salt)
3. 5 failed attempts protection
4. Factory reset option for forgotten passwords

⚠️ **Important:** Forgotten password requires factory reset (clears all data).

## 🎛️ Interface Controls
- **Dashboard/WebView Toggle** - Switch between management and browsing
- **Dark/Light Theme Toggle** - Switch visual themes
- **Profile Selector** - Choose shared, named-shared, or private

## ⚠️ Important Notes
- Do not log out before saving sessions if you want to retain login status
- Named-shared profiles share data across ALL tabs using that profile name, regardless of website
- Private profiles can be saved in sessions for repeated clean-slate use

## 🤝 Contributing

We welcome contributions! Please read our contributing guidelines and submit pull requests.

## 📄 License

GPL v3 License - see [LICENSE](LICENSE) file.

## 🙏 Credits
- **Qt Framework** (https://www.qt.io/) - Licensed under LGPL v3
- **Feather Icons** (https://feathericons.com/) - MIT License, Copyright (c) 2013-2017 Cole Bemis

---

**Copyright © 2025 Alamahant**  
Made with ❤️ for productivity enthusiasts, multi-account managers, and media lovers
