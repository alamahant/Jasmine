# Changelog

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




