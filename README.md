## ⚠️ **Disclaimer (免责声明)**  
This tool is **for educational/research purposes only**. It demonstrates Windows hook techniques and shared network operations.  
**DO NOT use it for unauthorized activities.** The developer is not responsible for misuse.  

## 🔧 **Features**  
- Captures keyboard input via DLL injection.  
- Logs keystrokes to a shared network folder (`\\IP\KeyLogs`).  
- Runs silently (no visible window).  

## 📝 **Usage**  
1. On the listener machine:  
   - Create a shared folder named `KeyLogs`.  
   - Grant write permissions to the user running this tool.  
2. Edit `config.txt` with the listener's IP/hostname.  
3. Run the executable on the target machine (for testing purposes).  

## ⚠️ **Legal & Ethical Notice**  
- This tool should only be used on **systems you own or have explicit permission to test**.  
- Many jurisdictions prohibit unauthorized keylogging. **Use at your own risk**.  