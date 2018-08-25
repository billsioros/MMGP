const {app, Menu, BrowserWindow, dialog} = require('electron')

// Keep a global reference of the window object, if you don't, the window will
// be closed automatically when the JavaScript object is garbage collected.
let win
let pythondir = __dirname + "/../../python/"
let datadir = __dirname + "/../../data/"
let DBFile = datadir + "MMGP_data.db"

function createWindow() {
    // Create the browser window.
    win = new BrowserWindow({width:1640, height:840, title:"MMGP", opacity: 1.0})
    //win.maximize();

    // and Load the index.html of the app
    win.loadFile("html/index.html")
    let search = require("../js/search.js");

    var menu = Menu.buildFromTemplate([
        {
            label: 'Main Menu',
            submenu: [
                {label: 'Run MMGP Algorithm'},        
                {type: 'separator'},
                {label: 'Exit', accelerator: 'CmdOrCtrl+Shift+W', click() {app.quit()}}
            ]
        },
        {
            label: 'Database Management',
            submenu: [
                {label: 'Create Database (overwrite everything)', click() {CreateDatabase();}},
                {label: 'Backup Database', click() {BackupDatabase();}},
                {type: 'separator'},
                {label: 'Update Students', click() {UpdateStudents();}},
                {label: 'Update Buses', click() {UpdateBuses();}},
                {type: 'separator'},

                {label: 'Update All Distances (approx. 25min)', click() {UpdateAllDistances();}},
                {label: 'Calculate All Distances to File (approx. 25min)', click() {AllDistancesToFile();}},
                {label: 'Update All Distances From File', click() {FileToAllDistances();}},
                {type: 'separator'},

                {label: 'Update Morning Distances (approx. 25min)', click() {UpdateSpecificDistances("Morning");}},
                {label: 'Calculate Morning Distances to File (approx. 25min)', click() {SpecificDistancesToFile("Morning");}},
                {label: 'Update Morning Distances From File', click() {FileToSpecificDistances("Morning");}},
                {type: 'separator'},

                {label: 'Update Noon Distances (approx. 25min)', click() {UpdateSpecificDistances("Noon");}},
                {label: 'Calculate Noon Distances to File (approx. 25min)', click() {SpecificDistancesToFile("Noon");}},
                {label: 'Update Noon Distances From File', click() {FileToSpecificDistances("Noon");}},
                {type: 'separator'},

                {label: 'Update Study Distances (approx. 25min)', click() {UpdateSpecificDistances("Study");}},
                {label: 'Calculate Study Distances to File (approx. 25min)', click() {SpecificDistancesToFile("Study");}},
                {label: 'Update Study Distances From File', click() {FileToSpecificDistances("Study");}},
                {type: 'separator'},
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
    var proc = spawn('python', [pythondir + "Creation.py", datadir + "Credentials.csv", "0", DBFile] );

    proc.on('close', function(code) {
        progressBar.setCompleted();
    })

    proc.stdout.on('data', function(data) {
        console.log(data.toString());
    })

    proc.stderr.on('data', function(data) {
        console.log(data.toString());
    })
}

function BackupDatabase() {
    
}

function UpdateStudents() {
    console.log(__dirname);
    const ProgressBar = require('electron-progressbar');

    var progressBar = new ProgressBar({
        title: "Updating Students..",
        text: "Updating Students..",
        detail: "Please Wait. This will not take long.\nDo not close this application! It is highly advised to not use the application right now.."
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
    var proc = spawn('python', [pythondir + "UpdateStudents.py", datadir + "Credentials.csv", "0", DBFile]);

    proc.on('close', function(code) {
        progressBar.setCompleted();
    })

    proc.stderr.on('data', function(data) {
        console.log(data.toString());
    })
}

function UpdateBuses() {
    const ProgressBar = require('electron-progressbar');

    var progressBar = new ProgressBar({
        title: "Updating Buses..",
        text: "Updating Buses..",
        detail: "Please Wait. This will not take long.\nDo not close this application! It is highly advised to not use the application right now.."
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
    var proc = spawn('python', [pythondir + "UpdateBuses.py", datadir + "Credentials.csv", "0", DBFile]);

    proc.on('close', function(code) {
        progressBar.setCompleted();
    })

    proc.stderr.on('data', function(data) {
        console.log(data.toString());
    })
}

function UpdateAllDistances() {
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

            thidistproc[0].stderr.on('data', function(data) {
                console.log(data.toString());
            })
        })

        secdistproc[0].stderr.on('data', function(data) {
            console.log(data.toString());
        })
    })

    firstdistproc[0].stderr.on('data', function(data) {
        console.log(data.toString());
    })
}

function AllDistancesToFile() {

    dialog.showOpenDialog((fileNames) => {
        if (!fileNames) {
            console.log("undefined filenames")
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
    
                thidistproc[0].stderr.on('data', function(data) {
                    console.log(data.toString());
                })
            })
    
            secdistproc[0].stderr.on('data', function(data) {
                console.log(data.toString());
            })
        })
    
        firstdistproc[0].stderr.on('data', function(data) {
            console.log(data.toString());
        })

    })

}

function FileToAllDistances() {

}

function UpdateSpecificDistances(DayPart) {
    let proc = UpdateDayPartDistances(DayPart, true)

    proc[0].on('close', function() {
        proc[1].setCompleted()
        console.log("Updating " + DayPart + " Distances completed.")
    })

    proc[0].stderr.on('data', function(data) {
        console.log(data.toString());
    })
}

function SpecificDistancesToFile(DayPart) {
    dialog.showOpenDialog((fileNames) => {
        if (!fileNames) {
            console.log("undefined filenames")
            return;
        }
        
        var fileName = fileNames[0]

        let proc = UpdateDayPartDistances(DayPart, false, fileName)

        proc[0].on('close', function() {
            proc[1].setCompleted();
            console.log("Updating " + DayPart + " Distances to file: " + fileName + " completed.")
        })
    
        proc[0].stderr.on('data', function(data) {
            console.log(data.toString());
        })

    })
}

function UpdateDayPartDistances(DayPart, direct=false, fileName=undefined) {
    const spawn = require("child_process").spawn;
    var updistproc;

    if (direct)
        updistproc = spawn('python', [pythondir + "UpdateDistances.py", datadir + "Credentials.csv", "0", DayPart, DBFile, "-d"]);
    else {
        if (fileName)
            updistproc = spawn('python', [pythondir + "UpdateDistances.py", datadir + "Credentials.csv", "0", DayPart, DBFile, fileName]);
        else
            updistproc = spawn('python', [pythondir + "UpdateDistances.py", datadir + "Credentials.csv", "0", DayPart, DBFile]);
    }

    var progressBar = undefined

    const ProgressBar = require('electron-progressbar');

    var progressBar = new ProgressBar({
        title: "Updating " + DayPart + " Distances..",
        text: "Updating " + DayPart + " Distances..",
        detail: "Please Wait. This will take some time.\nDo not close this application! It is highly advised to not use the application right now.."
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



