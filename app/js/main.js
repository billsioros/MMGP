const {app, Menu, BrowserWindow, dialog, ipcMain, shell, os} = require('electron')


// #region Variables

// Keep a global reference of the window object, if you don't, the window will
// be closed automatically when the JavaScript object is garbage collected.

let win
let printwin
let pythondir = __dirname + "/../../python/"
let datadir = __dirname + "/../../data/"
let Settings = datadir + "MMGP_Settings.json"
let DBFile = datadir + "MMGP_data.db"
let ScheduleChangesFile = datadir + "ScheduleChanges.json"
let closing = false

let OpenProcesses = {};

// #endregion   //



// #region Window Handlers  //

function initPrintWindow() {
    if (closing) {
        printwin = null;
    }
    printwin = new BrowserWindow({
        width:1640, 
        height:840, 
        title:"MMGP_Print", 
        opacity: 1.0,
        frame: false});
    printwin.loadFile("html/printer.html")
    printwin.maximize();
    printwin.hide();
    // printwin.webContents.openDevTools();
    printwin.on("closed", () => {
        if (closing) {
            printwin = null;
        }
        else {
            initPrintWindow();
        }
    });
}

function createWindow() {
    // Create the browser window.
    win = new BrowserWindow({width:1640, height:840, title:"MMGP", opacity: 1.0})
    win.maximize();

    initPrintWindow();

    let fs = require("fs");
    let data = fs.readFileSync(Settings)
    let SettingsObj = JSON.parse(data)

    let native = false;
    
    if (SettingsObj.Connection.Active === "Native") {
        native = true;
    }

    let oldYear = false;
    let newYear = false;
    let bothYears = false;
    if( SettingsObj.Current_Year.Active === "Old") {
        oldYear = true;
    }
    else if (SettingsObj.Current_Year.Active === "New") {
        newYear = true;
    }
    else {
        bothYears = true;
    }

    // and Load the index.html of the app
    win.loadFile("html/index.html")
    
    var menu = Menu.buildFromTemplate([
        {
            label: 'Main Menu',
            submenu: [
                {label: 'Run MMGP Algorithm'},        
                {type: 'separator'},
                {label: 'Hide Print Window', click() {printwin.hide()}},
                {label: 'Exit', accelerator: 'CmdOrCtrl+Shift+W', click() {KillSubProcesses(); app.quit()}}
            ]
        },
        {
            label: "Connection - Year",
            submenu: [
                {type: "radio", checked: native, label: 'Native Client' , click() {setActiveConnection("Native");}},
                {type: "radio", checked: !native, label: 'Network Client', click() {setActiveConnection("Network");}},
                {type:"separator"},
                {type: "radio", checked: oldYear, label: '"Old" Year', click() {setActiveCurrentYear("Old"); UpdateStudents(true);}},
                {type: "radio", checked: newYear, label: '"New" Year', click() {setActiveCurrentYear("New"); UpdateStudents(true);}},
                {type: "radio", checked: bothYears, label: 'Both "Old" and "New" Year', click() {setActiveCurrentYear("Both"); UpdateStudents(true);}}
            ]
        },
        {
            label: 'Database Management',
            submenu: [
                {label: 'Create Database (overwrite everything)', accelerator: "CmdOrCtrl+Shift+D" ,click() {CreateDatabase();}},
                {label: 'Backup Database', click() {BackupDatabase();}},
                {label: 'Restore Database', click() {RestoreDatabase();}},
                {type: 'separator'},
                {label: 'Update Students', accelerator: "CmdOrCtrl+Shift+U", click() {UpdateStudents(false);}},
                {label: 'Update Students (overwrite current addresses)', click() {UpdateStudents(true);}},
                {label: 'Update Buses', click() {UpdateBuses(); win.reload(); printwin.reload();}},
                {label: 'Insert Schedule Changes', click() {InsertScheduleChanges();}},
                {type: 'separator'},

                {label: 'Update All Distances (approx. 25min)', click() {UpdateAllDistances();}},
                {label: 'Calculate All Distances to File (approx. 25min)', click() {AllDistancesToFile();}},
                {label: 'Update All Distances From File', click() {FileToAllDistances();}},
                {type: 'separator'},

                {label: 'Update Morning Distances (approx. 10min)', click() {UpdateSpecificDistances("Morning");}},
                {label: 'Calculate Morning Distances to File (approx. 10min)', click() {SpecificDistancesToFile("Morning");}},
                {label: 'Update Morning Distances From File', click() {FileToSpecificDistances("Morning");}},
                {type: 'separator'},

                {label: 'Update Noon Distances (approx. 10min)', click() {UpdateSpecificDistances("Noon");}},
                {label: 'Calculate Noon Distances to File (approx. 10min)', click() {SpecificDistancesToFile("Noon");}},
                {label: 'Update Noon Distances From File', click() {FileToSpecificDistances("Noon");}},
                {type: 'separator'},

                {label: 'Update Study Distances (approx. 30sec)', click() {UpdateSpecificDistances("Study");}},
                {label: 'Calculate Study Distances to File (approx. 30sec)', click() {SpecificDistancesToFile("Study");}},
                {label: 'Update Study Distances From File', click() {FileToSpecificDistances("Study");}},
                {type: 'separator'},
            ]
        },
        {
            label: 'Window',
            submenu: [
                {label: 'Reload', accelerator: 'CmdOrCtrl+R', click() {
                    win.reload();
                    printwin.reload();
                    printwin.hide();
                }},
                {label: 'Debug', accelerator: 'CmdOrCtrl+Shift+I', click() {win.toggleDevTools(); printwin.webContents.toggleDevTools();}},
                {type: 'separator'},
                {label: 'Close Current Tab', accelerator: "CmdOrCtrl+T" ,click() {win.webContents.send("CloseTab");} },
                {label: 'Close All Tabs', accelerator: "CmdOrCtrl+Shift+T" ,click() {win.webContents.send("CloseAllTabs");} },         
                {label: 'Print Current Tab', accelerator: "CmdOrCtrl+P", click() {win.webContents.send("Print");} },
                {type: 'separator'},
                {label: 'Edit Current Student', accelerator: "CmdOrCtrl+E", click() {win.webContents.send("Edit");} }
            ]
        }
    ])
    win.setMenu(menu)

    // Emitted when the window is closed.
    win.on('closed', () => {
        // Dereference the window object, usually you would store windows
        // in an array if your app supports multi windows, this is the time
        // when you should delete the corresponding element.
        win = null
        closing = true;
        printwin.close();
        KillSubProcesses();
        app.quit();
    })
}

// #endregion



// #region Create - Backup - Restore    //

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


    let fs = require("fs");
    fs.unlink(DBFile, (err) => {
        if (err)
            console.error(err)
    })

    jsonfile = datadir + "tmp/createdatabase.json"

    let toJson = {    
        Settings: Settings,
        Database: DBFile
    }
    console.log('toJson', JSON.stringify(toJson));

    spawn = require("child_process").spawn;
    var proc = spawn('python', [pythondir + "Creation.py", JSON.stringify(toJson)]);
    OpenProcesses[proc.pid] = proc;

    proc.on('close', function(code) {
        progressBar.setCompleted();
    })

    proc.stdout.on('data', function(data) {
        console.log(data.toString());
    })

    proc.stderr.on('data', function(data) {
        dialog.showErrorBox("Creation Error", data.toString());
        console.error(data.toString());
    })

    
}

function BackupDatabase() {
    spawn = require("child_process").spawn;

    const ProgressBar = require('electron-progressbar');

    var progressBar = new ProgressBar({
        title: "Backing up Database..",
        text: "Backing up Database..",
        detail: "Please Wait. This will not take long.\nDo not close this application!"
    });

    progressBar.on('completed', function() {
        console.log("Successfully backed database.");
        progressBar.detail = "Backing up completed..";
    });
    progressBar.on('aborted', function() {
        console.log("Backing up Database canceled.")
        progressBar.detail = "Canceling.."
    })


    let fs = require('fs')
    jsonfile = datadir + "tmp/backupdatabase.json"

    let toJson = {
        Settings: Settings,
        Database: DBFile
    }

    spawn = require("child_process").spawn;
    var proc = spawn('python', [pythondir + "BackupDatabase.py", JSON.stringify(toJson)]);
    OpenProcesses[proc.pid] = proc;

    proc.on('close', function(code) {
        progressBar.setCompleted();
    })

    proc.stdout.on('data', function(data) {
        console.log(data.toString());
    })

    proc.stderr.on('data', function(data) {
        dialog.showErrorBox("Backup Error", data.toString());
        console.error(data.toString());
    })
}

function RestoreDatabase() {
    let fs = require('fs');

    dialog.showOpenDialog({
        defaultPath: datadir, 
        filters: [
            {name: 'Databases', extensions: ['db']}
        ]}, (fileNames) => {
        if (!fileNames) {
            dialog.showErrorBox("Error", "Undefined Filename!")
            return;
        }

        spawn = require("child_process").spawn;

        const ProgressBar = require('electron-progressbar');

        var progressBar = new ProgressBar({
            title: "Restoring Database..",
            text: "Restoring Database..",
            detail: "Please Wait. This will not take long.\nDo not close this application!"
        });

        progressBar.on('completed', function() {
            console.log("Successfully restored database.");
            progressBar.detail = "Restoring completed..";
        });
        progressBar.on('aborted', function() {
            console.log("Restoring Database canceled.")
            progressBar.detail = "Canceling.."
        })

        jsonfile = datadir + "tmp/backupdatabase.json"

        let toJson = {
            Settings: Settings,
            Database: DBFile,
            Backup: fileNames[0]
        }

        spawn = require("child_process").spawn;
        var proc = spawn('python', [pythondir + "RestoreDatabase.py", JSON.stringify(toJson)]);
        OpenProcesses[proc.pid] = proc;

        proc.on('close', function(code) {
            progressBar.setCompleted();
        })

        proc.stdout.on('data', function(data) {
            console.log(data.toString());
        })

        proc.stderr.on('data', function(data) {
            dialog.showErrorBox("Restoring Error", data.toString());
        })
    })
}

// #endregion


// #region Update Students - Buses      //

function UpdateStudents(overwrite=false) {
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

    let fs = require("fs");

    jsonfile = datadir + "tmp/updatestudents.json"

    let toJson = {
        Settings: Settings,
        Database: DBFile,
        Overwrite: overwrite
    }


    spawn = require("child_process").spawn;
    var proc = spawn('python', [pythondir + "UpdateStudents.py", JSON.stringify(toJson)]);
    OpenProcesses[proc.pid] = proc;

    proc.on('close', function(code) {
        progressBar.setCompleted();
    })

    proc.stderr.on('data', function(data) {
        dialog.showErrorBox("Student Update Error", data.toString());
    })

    proc.stdout.on('data', function(data) {
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

    let fs = require("fs");

    jsonfile = datadir + "tmp/updatebuses.json"

    let toJson = {
        Settings: Settings,
        Database: DBFile
    }

    spawn = require("child_process").spawn;
    var proc = spawn('python', [pythondir + "UpdateBuses.py", JSON.stringify(toJson)]);
    OpenProcesses[proc.pid] = proc;

    proc.on('close', function(code) {
        progressBar.setCompleted();
    })

    proc.stderr.on('data', function(data) {
        dialog.showErrorBox("Buses Update Error", data.toString());
        console.error(data.toString());
    })

    proc.stdout.on('data', function(data) {
        console.log(data.toString());
    })
}

function InsertScheduleChanges() {
    const spawn = require("child_process").spawn;

    let toJson = {}
    toJson.Database = DBFile;
    toJson.JSONFile = ScheduleChangesFile;
    toJson.Settings = Settings

    let proc = spawn('python', [pythondir + "InsertScheduleChanges.py", JSON.stringify(toJson)]);

    const ProgressBar = require('electron-progressbar');

    var progressBar = new ProgressBar({
        title: "Inserting..",
        text: "Inserting..",
        detail: "Please Wait. This will not take long.\nDo not close this application!"
    });

    progressBar.on('completed', function() {
        progressBar.detail = "Inserting Complete.";
    });

    proc.on('close', function(code) {
        console.log("Inserted.");
        progressBar.setCompleted();
    })

    proc.stdout.on('data', function(data) {
        console.log(data.toString());
    })

    proc.stderr.on('data', function(data) {
        dialog.showErrorBox("Inserting Error", data.toString());
    })
}

// #endregion   //


// #region Update All Distances //

function UpdateAllDistances() {
    let DayParts = ["Study", "Noon", "Morning"]

    let firstdistproc = UpdateDayPartDistances(DayParts[0], true)

    firstdistproc.process.on('close', function() {

        firstdistproc.progressBar.setCompleted();

        let secdistproc = UpdateDayPartDistances(DayParts[1], true)

        secdistproc.process.on('close', function() {

            secdistproc.progressBar.setCompleted();

            let thidistproc = UpdateDayPartDistances(DayParts[2], true)

            thidistproc.process.on('close', function() {

                thidistproc.progressBar.setCompleted();
            })

            thidistproc.process.stderr.on('data', function(data) {
                dialog.showErrorBox(DayParts[2] + "Distances Error", data.toString());
                console.error(data.toString());
            })
        })

        secdistproc.process.stderr.on('data', function(data) {
            dialog.showErrorBox(DayParts[1] + "Distances Error", data.toString());
            console.error(data.toString());
        })
    })

    firstdistproc.process.stderr.on('data', function(data) {
        dialog.showErrorBox(DayParts[0] + "Distances Error", data.toString());
        console.error(data.toString());
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

        firstdistproc.process.on('close', function() {
    
            firstdistproc.progressBar.setCompleted();
    
            let secdistproc = UpdateDayPartDistances(DayParts[1], false, fileName)
    
            secdistproc.process.on('close', function() {
    
                secdistproc.progressBar.setCompleted();
    
                let thidistproc = UpdateDayPartDistances(DayParts[2], false, fileName)
    
                thidistproc.process.on('close', function() {
    
                    thidistproc.progressBar.setCompleted();
                })
    
                thidistproc.process.stderr.on('data', function(data) {
                    dialog.showErrorBox(DayParts[2] + "Distances Error", data.toString());
                    console.error(data.toString());
                })

            })
    
            secdistproc.process.stderr.on('data', function(data) {
                dialog.showErrorBox(DayParts[1] + "Distances Error", data.toString());
                console.error(data.toString());
            })

        })
    
        firstdistproc.process.stderr.on('data', function(data) {
            dialog.showErrorBox(DayParts[1] + "Distances Error", data.toString());
            console.error(data.toString());
        })

    })

}

function FileToAllDistances() {

}

// #endregion


// #region Update Specific Distances //

function UpdateSpecificDistances(DayPart) {
    let proc = UpdateDayPartDistances(DayPart, true)

    proc.process.on('close', function() {
        proc.progressBar.setCompleted()
        console.log("Updating " + DayPart + " Distances completed.")
    })

    proc.process.stderr.on('data', function(data) {
        dialog.showErrorBox(DayPart + "Distances Error", data.toString());
    })

    proc.process.stdout.on('data', function(data) {
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

        proc.process.on('close', function() {
            proc.progressBar.setCompleted();
            console.log("Updating " + DayPart + " Distances to file: " + fileName + " completed.")
        })
    
        proc.process.stderr.on('data', function(data) {
            dialog.showErrorBox(DayPart + "Distances Error", data.toString());
            console.error(data.toString());
        })

    })
}

// #endregion


// #region Update Distances Handler //

function UpdateDayPartDistances(DayPart, direct=false, fileName=undefined) {
    const spawn = require("child_process").spawn;
    var updistproc;

    let fs = require("fs");

    jsonfile = datadir + "tmp/update" + DayPart + "distances.json"

    let toJson = {
        Settings: Settings,
        Database: DBFile,
        DayPart: DayPart,
        direct: direct,
    }

    if (fileName) {
        toJson.fileName = fileName
    }

    updistproc = spawn('python', [pythondir + "UpdateDistances.py", JSON.stringify(toJson)]);
    OpenProcesses[updistproc.pid] = updistproc;

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
    
    return {process: updistproc, progressBar: progressBar};
}

// #endregion


// #region DB Connection - TableNames handler   //

function setActiveConnection(con) {
    let fs = require("fs");

    var json_content;
    let raw_data = fs.readFileSync(Settings);
    let data = JSON.parse(raw_data)

    let toJson = data
    data.Connection.Active = con

    fs.writeFile(Settings, JSON.stringify(data), (err) => {
        if (err) {
            dialog.showErrorBox("File System Error", err);
            console.error(err);
            return;
        };
    });
}

function setActiveCurrentYear(year) {
    let fs = require("fs");

    var json_content;
    let raw_data = fs.readFileSync(Settings);
    let data = JSON.parse(raw_data)

    let toJson = data
    data.Current_Year.Active = year

    fs.writeFile(Settings, JSON.stringify(data), (err) => {
        if (err) {
            dialog.showErrorBox("File System Error", err);
            console.error(err);
            return;
        };
    });
}

// #endregion



// #region Electron App Handler //

// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
// Some APIs can only be used after this event occurs.
app.on('ready', createWindow)

// Quit when all windows are closed.
app.on('window-all-closed', () => {

    // On macOS it is common for applications and their menu bar
    // to stay active until the used quits explicitly with Cmd + Q
    
    if (process.platform !== 'darwin') {
        KillSubProcesses();
        app.quit()  
    }
})

app.on('activate', () => {
    // On macOS it's common to re-create a window in the app when the
    // dock icon is clicked and there are no other windows open.
    if (win === null) {
        createWindow
    }
})


function KillSubProcesses() {
    let pids = Object.keys(OpenProcesses);
    for (let i = 0; i < pids.length; i++) {
        OpenProcesses[pids[i]].kill();
    }
}
// #endregion



// #region Message Getter for printing  //

// retransmit it to printwin
ipcMain.on("printPDF", (event, content, title, type) => {
    printwin.show();
    printwin.webContents.send("printPDF", content, title, type);
});

// when worker window is ready
ipcMain.on("readyToPrintPDF", (event, title, type) => {

    dialog.showOpenDialog({
            defaultPath: datadir, 
            buttonLabel: "Select Folder",
            properties: [
                'openDirectory',
                'promptToCreate',
            ]
        }, 
        (fileNames) => {
            if (!fileNames) {
                printwin.hide();
                dialog.showErrorBox("Error", "Undefined Filename!")
                return;
            }

            const pdfPath = fileNames[0] + "/" + title + ".pdf"

            printwin.webContents.print({printBackground: true}, function (success) {
                if (!success) {
                    dialog.showErrorBox("Printing failed or canceled.")
                    printwin.hide();
                    return
                } 

                event.sender.send('wrote-pdf', pdfPath)
                printwin.hide();
            })
            printwin.hide();
        });
        
});

ipcMain.on("CanceledPrinting", (event) => {
    printwin.hide();
    event.sender.send('canceled-printing')
})

// #endregion   //


// #region Message Getter for Editing //

ipcMain.on("OpenStudentEditor", (event, Student) => {
    let editwindow = new BrowserWindow({
        width: 600, 
        height: 900, 
        title: Student.LastName + " " + Student.FirstName, 
        opacity: 1.0,
        autoHideMenuBar: true
    });


    var menu = Menu.buildFromTemplate([
        {
            label: 'File',
            submenu: [
                {label: 'Save', accelerator: 'CmdOrCtrl + S', click() {editwindow.webContents.send("Save")} },        
                {type: 'separator'},
                {label: 'Exit', accelerator: 'CmdOrCtrl+Shift+W', click() {KillSubProcesses(); editwindow.close()}}
            ]
        },
        {
            label: 'Window',
            submenu: [
                {label: 'Reload', accelerator: 'CmdOrCtrl+R', click() {
                    editwindow.reload();
                }},
                {label: 'Debug', accelerator: 'CmdOrCtrl+Shift+I', click() {editwindow.toggleDevTools();}},
            ]
        }
    ])
    editwindow.setMenu(menu)

    editwindow.loadFile("html/studenteditor.html");
    setTimeout(() => {
        editwindow.webContents.send("LoadStudent", Student);
    }, 1000);
    
})

ipcMain.on("Save", (event, Schedules, Student) => {
    const spawn = require("child_process").spawn;

    Schedules.Database = DBFile;
    Schedules.JSONFile = ScheduleChangesFile;
    Schedules.Settings = Settings

    let proc = spawn('python', [pythondir + "SaveSchedules.py", JSON.stringify(Schedules), "-k"]);

    const ProgressBar = require('electron-progressbar');

    var progressBar = new ProgressBar({
        title: "Saving..",
        text: "Saving ",
        detail: "Please Wait. This will not take long.\nDo not close this application!"
    });

    progressBar.on('completed', function() {
        progressBar.detail = "Saving Complete.";
        event.sender.send("SavedClose");
    });

    proc.on('close', function(code) {
        console.log("Saved.");
        progressBar.setCompleted();
    })

    proc.stdout.on('data', function(data) {
        console.log(data.toString());
    })

    proc.stderr.on('data', function(data) {
        dialog.showErrorBox("Saving Error", data.toString());
    })
})

// #endregion
