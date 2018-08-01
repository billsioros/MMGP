const {app, Menu, BrowserWindow} = require('electron')
const {BackClick, ForwardClick} = require('./search.js')

// Keep a global reference of the window object, if you don't, the window will
// be closed automatically when the JavaScript object is garbage collected.
let win


function createWindow() {
    // Create the browser window.
    win = new BrowserWindow({width:1600, height:800})
    //win.maximize();

    // and Load the index.html of the app
    win.loadFile("html/index.html")

    var menu = Menu.buildFromTemplate([
        {
            label: 'Menu',
            submenu: [
                {label: 'Run MMGP Algorithm'},                
                {type: 'separator'},
                {label: 'Exit', accelerator: 'CmdOrCtrl+Shift+W', click() {app.quit()}}
            ]
        },
        {
            label: 'Window',
            submenu: [
                {label: 'Reload', accelerator: 'CmdOrCtrl+R', click() {win.reload();}},
                {label: 'Debug', accelerator: 'CmdOrCtrl+Shift+I', click() {win.toggleDevTools();}},
                {label: 'Back', accelerator: 'PageUp', click() {BackClick();}},
                {label: 'Forward', accelerator: 'PageDown', click() {ForwardClick();}}
            ]
        }
    ])

    Menu.setApplicationMenu(menu);

    // Emitted when the window is closed.
    win.on('closed', () => {
        // Dereference the window object, usually you would store windows
        // in an array if your app supports multi windows, this is the time
        // when you should delete the corresponding element.
        win = null
    })
}

// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
// Some APIs can only be used after this event occurs.
app.on('ready', createWindow)

// Quit when all windows are closed.
app.on('window-all-closed', () => {

    // On macOS it is common for applications and their menu bar
    // to saty active until the used quits explicitly with Cmd + Q
    if (process.platform !== 'darwin') {
        app.quit
    }
})

app.on('activate', () => {
    // On macOS it's common to re-create a window in the app when the
    // dock icon is clicked and there are no other windows open.
    if (win === null) {
        createWindow
    }
})



