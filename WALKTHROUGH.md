# ParaFetch Feature Walkthrough

I have successfully added a suite of powerful new features to ParaFetch, transforming it into a full-featured download manager.

## New Features

### 1. Settings Management
A new **Settings Dialog** (accessible via the Toolbar) allows you to configure:
- **Default Download Path**: Choose where files are saved automatically.
- **Connection Limits**: Set the default number of parallel connections (1-32).
- **Speed Limits**: Set a global speed limit (e.g., 1 MB/s) to avoid hogging bandwidth.
- **Behavior**: Toggle auto-start, clipboard monitoring, and system tray icon.

### 2. Batch Downloads
You can now import multiple URLs at once!
- Click **"📋 Batch Import"** in the toolbar.
- Paste a list of URLs or load them from a text file.
- ParaFetch will validate them and queue them all up instantly.

### 3. Bandwidth Limiter
- Control your download speed with precision.
- Set limits in the Settings dialog (e.g., "512 KB/s", "10 MB/s", or Custom).
- The limit is applied dynamically to all active downloads.

### 4. Browser Integration
- **Clipboard Monitoring**: If enabled in Settings, ParaFetch detects when you copy a URL and can automatically handle it.
- **Drag & Drop**: Simply drag a link from your browser and drop it onto the ParaFetch window to start a download.

### 5. Desktop Notifications
- Get notified via system bubbles when a download **completes** or **fails**.
- Works even if the ParaFetch window is minimized.

### 6. System Tray & Context Menus
- **System Tray**: Minimize ParaFetch to the tray to keep it running in the background.
- **Context Menu**: Right-click any download to:
  - Open the file
  - Open the folder
  - Copy the URL
  - Remove the task

## How to Test

1. **Build the project**:
   ```bash
   cd build
   cmake ..
   make
   ./ParaFetch
   ```

2. **Try Batch Download**:
   - Create a file `urls.txt` with some links.
   - Click "Batch Import" and select the file.

3. **Test Speed Limit**:
   - Go to Settings -> Downloads.
   - Set Speed Limit to "1 MB/s".
   - Start a large download and watch the speed graph cap at ~1 MB/s.

4. **Test Drag & Drop**:
   - Drag a link from Chrome/Firefox into the ParaFetch window.
