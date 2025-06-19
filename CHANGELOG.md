# Changelog

## [v1.2.0] - 2025-06-20
### Major Features and Improvements
- **Named Shared Profiles**: Implemented contextual browsing environments allowing multiple tabs to share the same browsing context under meaningful labels like "Work," "Home," or "Research"
  - Create and manage multiple named profiles via toolbar dropdown
  - Each profile maintains separate cookies, cache, and browsing data
  - Perfect for organizing different areas of digital life while allowing related tools to communicate
  - Easily switch between profiles with the profile selector in toolbar
  
- **Integrated Ad Blocker**: Added comprehensive ad blocking functionality
  - Toggle ad blocking on/off via Tools menu
  - Blocks common advertising domains and scripts
  - Improves page load times and reduces bandwidth usage
  - Works across all profiles (named, private, and default)

- **Enhanced Browser Privacy**:
  - Implemented custom request interceptor with standardized headers
  - Configured secure default headers for all web requests
  - Improved user agent consistency across browsing sessions

- **Profile Management Tools**:
  - Added profile manager dialog for creating, deleting, and cleaning Named Shared Profiles
  - Implemented named profiles data cleaning functionality to remove cookies and cache
  - Added safety checks to prevent deleting named profiles in use by active sessions
  - Improved named profile selection UI with clear visual indicators

- **UI Improvements**:
  - Added visual indicators for active Named profile
  - Improved toolbar layout with Named profile selector
  - Enhanced profile switching experience
  - Better integration between private toggle and named profiles

---

## [v1.1.0] - 2025-06-12
- Implemented a new symlink-based storage system that dramatically reduces disk space usage
- Profile data now uses symbolic links instead of copying entire directories, improving performance, speed and efficiency
- Added progress bar when saving sessions for better visual feedback and to prevent premature closure
- Implemented automatic cleanup of orphaned profile directories at startup (configurable in Tools menu)
- Added Help Menu documentation explaining the new storage system benefits and recommendations
- Performed minor code polishes

---

## [v1.0.1] - 2025-06-07
### Enhanced UI and functionality improvements
- Added fully functional URL bar with toggle visibility via eye icon in toolbar
- New feature to create website cards directly from the currently loaded URL
- Set dashboard to fixed size while keeping web view resizable for improved visual quality
- Fixed website card update button connection to proper slot function
- Improved theme handling for help menu items with temporary light theme switching
- Fixed critical QWebEngineView resizing bugs affecting display and performance
- Enhanced overall application layout with better size management
- Changed App summary to "Launch websites and sessions" uncapitilized to conform with Flathub's Quality Guidelines
- Changed default screenshot to "Main interface displaying organized bookmarks" which is a light theme screenshot to also conform with Flathub's Quality Guidelines

---

## [v1.0.0] - 2025-06-01
### Initial release of Jasmine Website and Session Manager
- Website and session management with organized card-based interface
- Complete session management with multi-tab restoration
- Private profile system for isolated browsing environments
- Multi-account support for simultaneous access to same services
- Built-in download manager with progress tracking
- Two-factor authentication (2FA) code generator
- Screenshot capture functionality
- Master password protection with SHA-256 encryption
- Data management and privacy controls
- Cross-platform Qt-based interface




