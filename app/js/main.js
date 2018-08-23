const {app, Menu, BrowserWindow, dialog} = require('electron')
const {BackClick, ForwardClick} = require('./search.js')

// Keep a global reference of the window object, if you don't, the window will
// be closed automatically when the JavaScript object is garbage collected.
let win


function createWindow() {
    // Create the browser window.
    win = new BrowserWindow({width:1640, height:840})
    //win.maximize();

    // and Load the index.html of the app
    win.loadFile("html/index.html")
    let search = require("../js/search.js");

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
            label: 'Edit',
            submenu: [
                {label: 'Create Database (overwrite everything)', click() {CreateDatabase();}},
                {type: 'separator'},
                {label: 'Update Students', click() {UpdateStudents();}},
                {label: 'Update Buses', click() {UpdateBuses();}},
                {label: 'Update Distances (approx. 25min)', click() {UpdateDistances();}},
                {label: 'Calculate Distances to File (approx. 25min)', click() {DistancesToFile();}}
            ]
        },
        {
            label: 'Window',
            submenu: [
                {label: 'Reload', accelerator: 'CmdOrCtrl+R', click() {win.reload();}},
                {label: 'Debug', accelerator: 'CmdOrCtrl+Shift+I', click() {win.toggleDevTools();}},
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

function CreateDatabase() {
    spawn = require("child_process").spawn;

    const ProgressBar = require('electron-progressbar');

    var progressBar = new ProgressBar({
        title: "Creating Database..",
        text: "Creating Database..",
        detail: "Please Wait. This will probably take a long time.\nGo grab a coffee and do not close this application!"
    });

    progressBar.on('completed', function() {
        console.log("Successfully created database.");
        progressBar.detail = "Creating completed..";
    });
    progressBar.on('aborted', function() {
        console.log("Creating Database canceled.")
        progressBar.detail = "Canceling.."
    })

    spawn = require("child_process").spawn;
    var proc = spawn('python', ["../python/Creation.py", "../data/Credentials.csv", "1"] );

    proc.on('close', function(code) {
        progressBar.setCompleted();
    })

    proc.stdout.on('data', function(data) {
        console.log(data.toString());
    })
}

function UpdateStudents() {
    const ProgressBar = require('electron-progressbar');

    var progressBar = new ProgressBar({
        title: "Updating Students..",
        text: "Updating Students..",
        detail: "Please Wait. This will not take long.\nDo not close this application!"
    });

    progressBar.on('completed', function() {
        console.log("Successfully updated students.");
        progressBar.detail = "Updating completed..";
    });
    progressBar.on('aborted', function() {
        console.log("Updating students canceled.")
        progressBar.detail = "Canceling.."
    })

    spawn = require("child_process").spawn;
    var proc = spawn('python', ["../python/UpdateStudents.py", "../data/Credentials.csv", "1"]);

    proc.on('close', function(code) {
        progressBar.setCompleted();
    })
}

function UpdateBuses() {
    const ProgressBar = require('electron-progressbar');

    var progressBar = new ProgressBar({
        title: "Updating Buses..",
        text: "Updating Buses..",
        detail: "Please Wait. This will not take long.\nDo not close this application!"
    });

    progressBar.on('completed', function() {
        console.log("Successfully updated buses.");
        progressBar.detail = "Updating completed..";
    });
    progressBar.on('aborted', function() {
        console.log("Updating Buses canceled.")
        progressBar.detail = "Canceling.."
    })

    spawn = require("child_process").spawn;
    var proc = spawn('python', ["../python/UpdateBuses.py", "../data/Credentials.csv", "1"]);

    proc.on('close', function(code) {
        progressBar.setCompleted();
    })
}

function UpdateDistances() {
    let DayParts = ["Study", "Noon", "Morning"]

    let firstdistproc = UpdateDayPartDistances(DayParts[0], true)

    firstdistproc[0].on('close', function() {

        firstdistproc[1].setCompleted();

        let secdistproc = UpdateDayPartDistances(DayParts[1], true)

        secdistproc[0].on('close', function() {

            secdistproc[1].setCompleted();

            let thidistproc = UpdateDayPartDistances(DayParts[2], true)

            thidistproc[0].on('close', function() {

                thidistproc[1].setCompleted();
            })
        })
    })
}

function DistancesToFile() {
    const fs = require("fs");

    dialog.showOpenDialog((fileNames) => {
        if (!fileNames) {
            console.log("undefined filename")
            return;
        }
        
        var fileName = fileNames[0]
        
        let DayParts = ["Study", "Noon", "Morning"]

        let firstdistproc = UpdateDayPartDistances(DayParts[0], false, fileName)
    
        firstdistproc[0].on('close', function() {
    
            firstdistproc[1].setCompleted();
    
            let secdistproc = UpdateDayPartDistances(DayParts[1], false, fileName)
    
            secdistproc[0].on('close', function() {
    
                secdistproc[1].setCompleted();
    
                let thidistproc = UpdateDayPartDistances(DayParts[2], false, fileName)
    
                thidistproc[0].on('close', function() {
    
                    thidistproc[1].setCompleted();
                })
            })
        })

    })

}

function UpdateDayPartDistances(DayPart, direct=false, fileName=undefined) {
    const spawn = require("child_process").spawn;
    var updistproc;

    if (direct)
        updistproc = spawn('python', ["../python/UpdateDistances.py", "../data/Credentials.csv", "1", DayPart, "-d"]);
    else {
        if (fileName)
            updistproc = spawn('python', ["../python/UpdateDistances.py", "../data/Credentials.csv", "1", DayPart, fileName]);
        else
            updistproc = spawn('python', ["../python/UpdateDistances.py", "../data/Credentials.csv", "1", DayPart]);
    }

    var progressBar = undefined

    const ProgressBar = require('electron-progressbar');

    var progressBar = new ProgressBar({
        title: "Updating " + DayPart + " Distances..",
        text: "Updating " + DayPart + " Distances..",
        detail: "Please wait. This will take some time.. Do not close this application!"
    });

    progressBar.on('completed', function() {
        console.log("Successfully updated " + DayPart + " distances.");
        progressBar.detail = "Updating completed..";
    });
    
    return [updistproc, progressBar];
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



