// #region Variables        //

// External modules
let sqlite3;
let DOMElementHistory;
let spawn
let fs
const ipcRenderer = require("electron").ipcRenderer;

// TabGroups
let SearchTabGroup;

// Dom Elements
let InfoMapTabHeader;
let SearchTabHeader;
let MainInfo;
let docmain;

let currentOpenBusID = '0';
//let studentsToPlot;

let docheader;
let mainHistory;
let headerHistory;

let map;
let closing = false;

let pythondir = __dirname + "/../../python/"
let datadir = __dirname + "/../../data/"
let DBFile

let MarkerColors;
let MarkerURL;

// #endregion //



// #region Bus Searching        //


    // #region Side-bar Schedule Searching Buttons functions       //

function GetActiveDayPart() {
    let buttons = document.getElementsByClassName("DayPartSelectorButton");
    let active = null;

    for (let i = 0; i < buttons.length; i++) {
        if (buttons[i].classList.contains("active")) {
            active = buttons[i];
            break;
        }
    }

    return active;
}

function GetActiveSchedule() {
    let buttons = document.getElementsByClassName("ScheduleButton");
    let active = [];

    for (let i = 0; i < buttons.length; i++) {
        if (buttons[i].classList.contains("active")) {
            active.push(buttons[i]);
        }
    }

    return active
}

function GetActiveBus() {
    let buttons = document.getElementsByClassName("BusButton");
    let active = [];

    for (let i = 0; i < buttons.length; i++) {
        if (buttons[i].classList.contains("active")) {
            active.push(buttons[i]);
        }
    }

    return active
}

// Bus Search Button Generation //

function GenerateDayPartButtons() {
    let DayPartContainer = document.getElementById("DayPartSelectorButtonsContainer");
    DayPartContainer.innerHTML = "";

    // We can be sure that we have at least one bus.
    sql = "Select distinct(DayPart) From Schedule Order By DayPart ASC"

    let searchobj = ExecuteSQLToProc(sql, function() {
        
        let json_content;
        let raw_data = fs.readFileSync(searchobj.json_file);
        let raw_students = JSON.parse(raw_data)
        let Rows = raw_students.Rows;

        for (let i = 0; i < Rows.length; i++) {
            let button = document.createElement("button");
            button.className = "DayPartSelectorButton SelectorButton";
            button.innerHTML = Rows[i].DayPart;
            button.onclick = OnDayPartClick;
            DayPartContainer.appendChild(button);
        }

        fs.unlink(searchobj.json_file, (err) => {
            if (err) {
                alert(err);
                console.error(err)
            }
        })
    })
}

function GenerateScheduleButtons() {
    let ScheduleContainer = document.getElementById("ScheduleSelectorContainer");
    ScheduleContainer.innerHTML = "";

    let DayPart = GetActiveDayPart().innerHTML;
    let sql;

    switch(DayPart) {
        case "Morning":
            sql = "Select distinct(BusSchedule) From Schedule Where Length(BusSchedule) > 1 and BusSchedule Like '%Π%'";
            break;
        case "Noon":
            sql = "Select distinct(BusSchedule) From Schedule Where Length(BusSchedule) > 1 and BusSchedule Like '%Μ%'"
            break;
        case "Study":
            sql = "Select distinct(BusSchedule) From Schedule Where Length(BusSchedule) = 1 Order By BusSchedule"
            let buses = GetActiveBus();
            for (let i = 0; i < buses.length; i++)
                buses[i].classList.remove("active")
            break;
        default: break;
    }


    let searchobj = ExecuteSQLToProc(sql, function() {
        
        let json_content;
        let raw_data = fs.readFileSync(searchobj.json_file);
        let raw_students = JSON.parse(raw_data)
        let Rows = raw_students.Rows;

        if (DayPart === "Study") {
            
            for (let i = 0; i < Rows.length; i++) {
                let row = Rows[i];
                const newButton = document.createElement("button");
                newButton.type = "button";
                newButton.className = "ScheduleButton";
                newButton.innerHTML = row.BusSchedule;
                newButton.onclick = OnScheduleClick;

                ScheduleContainer.appendChild(newButton);
            }
        }
        else {
            let distinctSchedules = []
            for (let i = 0; i < Rows.length; i++) {
                let row = Rows[i];
                if (!distinctSchedules.includes(row.BusSchedule[3])) {
                    distinctSchedules.push(row.BusSchedule[3])
                }
            }

            
            for (let i = 0; i < distinctSchedules.length; i++) {
                const newButton = document.createElement("button");
                newButton.type = "button";
                newButton.className = "ScheduleButton";
                newButton.innerHTML = i + 1;
                newButton.onclick = OnScheduleClick;

                ScheduleContainer.appendChild(newButton);
            }
        }

        fs.unlink(searchobj.json_file, (err) => {
            if (err) {
                alert(err);
                console.error(err)
            }
        })

    })
}

function GenerateBusButtons() {
    const BusButtons = document.getElementById("BusButtonsContainer");

    let sql = "Select Number From Bus Order By Number";

    let searchobj = ExecuteSQLToProc(sql, function() {
        
        let json_content;
        let raw_data = fs.readFileSync(searchobj.json_file);
        let raw_students = JSON.parse(raw_data)
        let Rows = raw_students.Rows;

        for (let i = 0; i < Rows.length; i++) {
            let row = Rows[i];
            const newButton = document.createElement("button");
            newButton.type = "button";
            newButton.className = "BusButton";
            if (row.Number.toString().length < 2) {
                row.Number = "0" + row.Number.toString();
            }
            newButton.id = row.Number;
            newButton.innerHTML = row.Number;
            newButton.onclick = OnBusClick;

            BusButtons.appendChild(newButton);
        }

        fs.unlink(searchobj.json_file, (err) => {
            if (err) {
                alert(err);
                console.error(err)
            }
        })
    })
}

function GenerateBusDayPartButtons() {
    const BusButtons = document.getElementById("BusButtonsContainer");

    let sql = "Select Number From Bus Order By Number";

    let searchobj = ExecuteSQLToProc(sql, function() {
        
        let json_content;
        let raw_data = fs.readFileSync(searchobj.json_file);
        let raw_students = JSON.parse(raw_data)
        let Rows = raw_students.Rows;

        for (let i = 0; i < Rows.length; i++) {
            let row = Rows[i];
            const newButton = document.createElement("button");
            newButton.type = "button";
            newButton.className = "BusButton";
            if (row.Number.toString().length < 2) {
                row.Number = "0" + row.Number.toString();
            }
            newButton.id = row.Number;
            newButton.innerHTML = row.Number;
            newButton.onclick = OnBusClick;

            BusButtons.appendChild(newButton);
        }

        fs.unlink(searchobj.json_file, (err) => {
            if (err) {
                alert(err);
                console.error(err)
            }
        })

        GenerateDayPartButtons();

    })
}

// #endregion //


    // #region Click Handlers                   //

function OnBusClickHandle() {
    let buttons = document.getElementsByClassName("DayPartSelectorButton");
    
    for (let i = 0; i < buttons.length; i++) {
        buttons[i].onclick = OnDayPartClick;
    }

    let button = document.getElementById("ScheduleSearchButton");
    button.onclick = SearchSchedule;

    button = document.getElementById("AddScheduleButton");
    button.onclick = AddSchedule;

    button = document.getElementById("CalculateDurationButton");
    button.onclick = CalculateScheduleDuration;

    button = document.getElementById("ClearBusButton");
    button.onclick = OnClearBusClick;

}

function OnDayPartClick() {
    let prevActive = GetActiveDayPart();
    if (prevActive)
        prevActive.classList.remove("active");
    
    if (prevActive === this)
        return;
    this.classList.add("active");
    GenerateScheduleButtons();
}

function OnScheduleClick() {
    let prevActive = GetActiveSchedule();
        
    if (prevActive.includes(this)) {
        this.classList.remove("active");
        prevActive.splice(prevActive.indexOf(this), 1);
        return;
    }
    
    this.classList.add("active");
    prevActive.push(this);
}

function OnBusClick() {
    let prevActive = GetActiveBus();
        
    if (prevActive.includes(this)) {
        this.classList.remove("active");
        prevActive.splice(prevActive.indexOf(this), 1);
        return;
    }
    
    this.classList.add("active");
    prevActive.push(this);
}

function OnClearBusClick() {
    let prevActive = GetActiveDayPart();
    if (prevActive)
        prevActive.classList.remove("active");

    let ScheduleContainer = document.getElementById("ScheduleSelectorContainer");
    ScheduleContainer.innerHTML = "";
    
    prevActive = GetActiveBus()

    for (let i = 0; i < prevActive.length; i++) {
        let bus = prevActive[i];
        bus.classList.remove("active");
    }
}

function CheckDisabledScheduleButton(tab) {
    if (tab == null) {
        document.getElementById("AddScheduleButton").disabled = true;
        document.getElementById("CalculateDurationButton").disabled = true;
        document.getElementById("PrintButton").disabled = true;
        return;
    }

    if (tab.type === "Schedule") {
        document.getElementById("AddScheduleButton").disabled = false;
        document.getElementById("CalculateDurationButton").disabled = false;
        document.getElementById("PrintButton").disabled = false;
    }
    else {
        document.getElementById("AddScheduleButton").disabled = true;
        document.getElementById("CalculateDurationButton").disabled = true;
        document.getElementById("PrintButton").disabled = false;
    }
    return;
}

    // #endregion //


    // #region Schedule Search-Add-Calculate    //

function GetScheduleSearchCriteria() {
    let activeBus = null;
    let activeSchedule = null;
    let activeBusButtons = GetActiveBus();
    let activeDayPartButton = GetActiveDayPart()
    let activeSchedules = GetActiveSchedule();

    let busSchedules = []

    for (let j = 0; j < activeSchedules.length; j++) {
    
        activeSchedule = activeSchedules[j];

        let activeDayPart = activeDayPartButton.innerHTML;

        if (!activeDayPart || !activeSchedule) {
            alert("Error: No DayPart or Schedule given.");
            return;
        }

        let busSchedule;

        if (activeDayPart === "Study") 
            busSchedules.push(activeSchedule.innerHTML)

        else {
            for (let i = 0; i < activeBusButtons.length; i++) {

                activeBus = activeBusButtons[i].innerHTML
    
                switch(activeDayPart) {
                    case "Morning":
                        if (!activeBus) {
                            alert("Error: No Bus given.");
                            return;
                        }
                        busSchedule = activeBus + "Π" + activeSchedule.innerHTML;
                        break;
                    case "Noon":
                        if (!activeBus) {
                            alert("Error: No Bus given.");
                            return;
                        }
                        busSchedule = activeBus + "Μ" + activeSchedule.innerHTML;
                        break;
                    default: break;
                }
    
                busSchedules.push(busSchedule)
            }
        }
    }

    return busSchedules;
}

function SearchSchedule() {

    let busSchedules = GetScheduleSearchCriteria();
    if (busSchedules.length === 0)
        return;

    let loading = document.getElementById("ScheduleSearchButton").childNodes[1];
    loading.hidden = false;

    loading.nextElementSibling.innerHTML = "Searching";

    let sql = "Select * From Student, Address, Schedule Where Schedule.AddressID = Address.AddressID and Schedule.StudentID = Student.StudentID and (";

    for (let i = 0; i < busSchedules.length; i++) {
        let busSchedule = busSchedules[i];
        sql += "BusSchedule = \"" + busSchedule + "\" ";

        if (i + 1 < busSchedules.length) {
            sql += "or ";
        }
    }

    sql += ") Order by BusSchedule, ScheduleOrder"
    
    let searchobj = ExecuteSQLToProc(sql, function() {
        let Students = ScheduleJsonRead(searchobj.json_file);

        let title = "";
        for (let i = 0; i < busSchedules.length; i++) {
            title += busSchedules[i]
            if (i + 1 < busSchedules.length) {
                title += ", ";
            }
        }

        let waitTime = 20;
        let type = "Schedule";  

        let newSearchTab = OpenSearchTab(docmain, title, type, DisplayBusTable, DisplayBusMap, Students);
        newSearchTab.activate(false);
        newSearchTab.activeBuses = busSchedules;

        // Assign onclick to More Buttons.  //
        ReassignAllButtons(); 

        loading.hidden = true;
        loading.nextElementSibling.innerHTML = "Search Schedule(s)";
    });
}

function AddSchedule() {
    let activeTab = SearchTabGroup.activeTab()

    if (activeTab.type !== "Schedule") {
        return;
    }
    
    let busSchedules = GetScheduleSearchCriteria();
    let actualBusSchedules = []

    if (activeTab.hasOwnProperty("activeBuses")) {
        if (activeTab.activeBuses.length + busSchedules.length > MarkerColors.length) {
            alert("Can't have more than " + MarkerColors.length + " open schedules at once.");
            return;
        }

        for (let i = 0; i < busSchedules.length; i++) {
            let busSchedule = busSchedules[i];
            if (!activeTab.activeBuses.includes(busSchedule)) {
                actualBusSchedules.push(busSchedule)
            }
        }
    }

    if (actualBusSchedules.length == 0) {
        alert("Error: All schedules selected are open in this tab!");
        return;
    }

    busSchedules = actualBusSchedules

    let loading = document.getElementById("AddScheduleButton").childNodes[1];

    loading.hidden = false;

    loading.nextElementSibling.innerHTML = "Adding"

    let sql = "Select * From Student, Address, Schedule Where Schedule.AddressID = Address.AddressID and Schedule.StudentID = Student.StudentID and (";

    for (let i = 0; i < busSchedules.length; i++) {
        let busSchedule = busSchedules[i];
        sql += "BusSchedule = \"" + busSchedule + "\" ";

        if (i + 1 < busSchedules.length) {
            sql += "or ";
        }
    }

    sql += ") Order by BusSchedule, ScheduleOrder"

    let searchobj = ExecuteSQLToProc(sql, function() {
        let Students = ScheduleJsonRead(searchobj.json_file)

        let title = activeTab.title + ", "

        for (let i = 0; i < busSchedules.length; i++) {
            title += busSchedules[i]
            if (i + 1 < busSchedules.length) {
                title += ", ";
            }
        }

        let type = "Schedule";
        // let waitTime = 20;

        Students.push.apply(Students, activeTab.students);
        let newSearchTab = OpenSearchTab(docmain, title, type, DisplayBusTable, DisplayBusMap, Students);

        newSearchTab.activeBuses = activeTab.activeBuses;
        newSearchTab.activeBuses.push.apply(newSearchTab.activeBuses, busSchedules);
        

        newSearchTab.activate(false);
        activeTab.close();
        
        // Assign onclick to More Buttons.
        ReassignAllButtons(); 

        loading.hidden = true;
        loading.nextElementSibling.innerHTML = "Add Schedule(s)"
    });
}
 
function CalculateScheduleDuration() {
    let activeTab = SearchTabGroup.activeTab();
    let Students = activeTab.students;

    let loading = document.getElementById("CalculateDurationButton").childNodes[1];
    loading.hidden = false;

    loading.nextElementSibling.innerHTML = "Calculating"

    let toJson = {
        students: [],
        Database: DBFile
    }

    toRoute = []

    for (let i = 0; i < Students.length; i++) {
        let student = Students[i]
        toJson.students.push({
            studentId: student.ID,
            addressId: student.Address.AddressID,
            longitude: student.Address.Longitude,
            latitude: student.Address.Latitude,
            dayPart: student.DayPart,
            earliest: 0,
            latest: 0,
        })
    }

    let proc = spawn('python', [pythondir + "CalculateScheduleDuration.py", JSON.stringify(toJson)]);

    proc.on('close', function(code) {
        loading.hidden = true;
        loading.nextElementSibling.innerHTML = "Calculate Duration"
    })

    proc.stdout.on('data', function(data) {
        
        console.log("Calculating Completed");
        Results = JSON.parse(data.toString());

        let Distance = Results.Distance;
        let Duration = Results.Duration;

        let Minutes = Math.ceil(Duration / 60)
        let Seconds = Math.ceil(Duration % 60)

        alert("Distance: " + (Distance / 1000).toString() + "km\nDuration: " + Minutes.toString() + "min " + Seconds.toString() + "sec.");
        
    })

    proc.stderr.on('data', function(data) {
        alert(data.toString());
        console.error(data.toString());
    }) 

}

function SolveScheduleTSP(Students) {
    toRoute = []

    for (let i = 0; i < Students.length - 1; i++) {
        let student = Students[i]
        toRoute.push({
            timewindow: [0, 0],
            addressId: student.Address.AddressID,
            studentId: student.ID
        })
    }

    let route = require("../../addons/route/build/Release/route.node");

    route(DBFile, Students[0].DayPart, 7*3600, 30, { addressId: Students[0].Address.AddressID }, toRoute, function(err, data) {
        if (err) {
          alert(err);
          return;
        }
        console.log(data);
    });
}

    // #endregion //


    // #region Bus - Schedule Display           //

function DisplayBusTable(Students) {
    let table = document.createElement("div")
    table.className = "Table"


    // Create the first row of headers
    let firstRow = document.createElement("div")
    firstRow.className = "BusTableRow TableRow"

    let Headers = [' ', 'Schedule', 'Index', 'Order', 'Time', 'Last Name', 'First Name', 'Class - Level', 'Address', 'Note', 'Mon', 'Tue', 'Wen', 'Thu', 'Fri']

    for (let i = 0; i < Headers.length; i++) {
        let p = document.createElement("p")
        p.className = "RowHeader"
        p.innerHTML = Headers[i]
        firstRow.appendChild(p)
    }

    table.appendChild(firstRow);

    if (Students.length === 0) {
        let p = document.createElement("p");
        p.className = "RowData";
        p.innerHTML = "-";
        table.appendChild(p);
    }

    let index = 1
    let currentBusSchedule = null
    let odd = false;
    for (let i = 0; i < Students.length; i++) {
        let student = Students[i];
        if (currentBusSchedule !== student.BusSchedule) {
            currentBusSchedule = student.BusSchedule
            index = 1
        }
        let row = document.createElement("div")
        row.className = "BusTableRow TableRow";
        
        

        // Keep the ID in html level but hidden,
        // so it can be retrieved afterwards for the "More" button
        let p = document.createElement("p");
        p.className = "RowData";
        p.innerHTML = student.ID;
        p.hidden = true;
        row.appendChild(p);

        let button = document.createElement("button")
        button.type =  "button";
        button.className = "RowData MoreButton"
        if (odd)
            button.classList.add("OddRowData")
        let searchimg = document.createElement("img");
        searchimg.src = "../images/General/more.png";
        searchimg.className = "MoreButtonImage"
        button.appendChild(searchimg);
        row.appendChild(button);

        p = document.createElement("p")
        p.className = "RowData"
        if (odd)
            p.classList.add("OddRowData")
        p.innerHTML = student.BusSchedule;
        row.appendChild(p)

        p = document.createElement("p")
        p.className = "RowData"
        if (odd)
            p.classList.add("OddRowData")
        p.innerHTML = index
        row.appendChild(p)
        index++;

        p = document.createElement("p")
        p.className = "RowData"
        if (odd)
            p.classList.add("OddRowData")
        p.innerHTML = student.ScheduleOrder;
        row.appendChild(p)

        p = document.createElement("p")
        p.className = "RowData"
        if (odd)
            p.classList.add("OddRowData")
        p.innerHTML = student.ScheduleTime;
        row.appendChild(p)
        
        p = document.createElement("p")
        p.className = "RowData"
        if (odd)
            p.classList.add("OddRowData")
        p.innerHTML = student.LastName
        row.appendChild(p)

        p = document.createElement("p")
        p.className = "RowData"
        if (odd)
            p.classList.add("OddRowData")
        p.innerHTML = student.FirstName
        row.appendChild(p)

        p = document.createElement("p");
        p.className = "RowData";
        if (odd)
            p.classList.add("OddRowData")
        p.innerHTML = student.ClassLevel;
        row.appendChild(p);

        p = document.createElement("p")
        p.className = "RowData"
        if (odd)
            p.classList.add("OddRowData")
        p.innerHTML = student.Address.FullAddress
        row.appendChild(p)

        p = document.createElement("p")
        p.className = "RowData"
        if (odd)
            p.classList.add("OddRowData")
        if (student.Notes)
            p.innerHTML = student.Notes;
        else
            p.innerHTML = "-";
        row.appendChild(p)

        // Days
        let WeekDays = ['Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday'];
                
        for (let j = 0; j < WeekDays.length; j++) {
            p = document.createElement("p");
            p.className = "RowData";
            if (odd)
                p.classList.add("OddRowData")

            if (student.Days[WeekDays[j]] === 1)
                p.className += " OnDay";
            else 
                p.className += " OffDay";

            row.appendChild(p);
        }

        table.appendChild(row);
        odd = !odd
    }
    MainInfo.appendChild(table);
}

function DisplayBusMap(Students) {
    let index = 1
    let currentBusSchedule = null
    let studentsToPlot = [];

    for (let i = 0; i < Students.length; i++) {
        let student = Students[i];

        if (currentBusSchedule !== student.BusSchedule) {
            currentBusSchedule = student.BusSchedule
            index = 1
        }

        studentsToPlot.push({
            Name: student.Name,
            Schedules: [{
                ScheduleID: student.ScheduleID,
                Address: student.Address,
                Time: student.ScheduleTime,
                BusSchedule: student.BusSchedule
            }],
            Order: index.toString()
        })

        index++;
    }

    return studentsToPlot;
}

    // #endregion //


// #endregion                   //



// #region Student Searching    //


    // #region  Student Table-Map Display   //

function DisplayStudentSearchTable(Students) {

    // Display Students given in parameter in a table inside "MainInfo".
    let table = document.createElement("div");
    table.className = "Table";

    let firstRow = document.createElement("div");
    firstRow.className = "StudentTableRow TableRow";

    let Headers = [' ', 'Index', 'Last Name', 'First Name', 'ClassLevel'];

    // Use Headers above to create the first row of the table, showing column Names
    for (let i = 0; i < Headers.length; i++) {
        let p = document.createElement("p");
        p.className = "RowHeader";
        p.innerHTML = Headers[i];
        firstRow.appendChild(p);
    }

    table.appendChild(firstRow);

    
    StudentKeys = Object.keys(Students);
    if (StudentKeys.length === 0) {
        let p = document.createElement("p");
        p.className = "RowData";
        p.innerHTML = "-";
        table.appendChild(p);
    }
    // Basically for student in Students:
    let odd = false
    for (let i = 0; i < StudentKeys.length; i++) {
        key = StudentKeys[i];
        let row = document.createElement("div");
        row.className = "StudentTableRow TableRow";
        
        // Keep the ID in html level but hidden,
        // so it can be retrieved afterwards for the "More" button
        let p = document.createElement("p");
        p.className = "RowData";
        if (odd)
            p.classList.add("OddRowData")
        p.innerHTML = Students[key].ID;
        p.hidden = true;
        row.appendChild(p);

        // The "More" button
        let button = document.createElement("button");
        button.type =  "button";
        button.classList.add("RowData");
        button.classList.add("MoreButton");
        if (odd)
            button.classList.add("OddRowData")
        button.onclick = OnMorePress;
        let searchimg = document.createElement("img");
        searchimg.src = "../images/General/more.png";
        searchimg.className = "MoreButtonImage";
        button.appendChild(searchimg);
        row.appendChild(button);

        // Index
        p = document.createElement("p");
        p.className = "RowData";
        if (odd)
            p.classList.add("OddRowData")
        p.innerHTML = i + 1;
        row.appendChild(p);
      
        // Last Name
        p = document.createElement("p");
        p.className = "RowData";
        if (odd)
            p.classList.add("OddRowData")
        p.innerHTML = Students[key].LastName;
        row.appendChild(p);

        // First Name
        p = document.createElement("p");
        p.className = "RowData";
        if (odd)
            p.classList.add("OddRowData")
        p.innerHTML = Students[key].FirstName;
        row.appendChild(p);

        // Class - Level
        p = document.createElement("p");
        p.className = "RowData";
        if (odd)
            p.classList.add("OddRowData")
        p.innerHTML = Students[key].ClassLevel;
        row.appendChild(p);

        table.appendChild(row);
        odd = !odd;
    }
        
    MainInfo.appendChild(table)
}

function DisplayStudentSearchMap(Students) {
    // This does not actually display map. It just saves info for when map will be displayed
    let studentsToPlot = [];

    StudentKeys = Object.keys(Students);
    for (let i = 0; i < StudentKeys.length; i++) {
        // Find the id = key to Students "dict"
        let student = Students[StudentKeys[i]];
        let id = "\"" + student.ID + "\"";

        // *Subject to change* //
        // Add all addresses to one array, so it is easier to plot them all together.
        let Schedules = [];
        Schedules.push.apply(Schedules, student.MorningSchedules);
        Schedules.push.apply(Schedules, student.NoonSchedules);
        Schedules.push.apply(Schedules, student.StudySchedules);

        studentsToPlot.push({
            Name: student.LastName + ' ' + student.FirstName,
            Schedules: Schedules,
            Order: ""
        });
    }

    return studentsToPlot;
}

function DisplayStudentCard(student) {
    // Card is divided in 3 sections: Info, Phones and Schedules
    let StCard = document.createElement("div")
    StCard.className = "StudentCard"

    // General Info
    {
        // General Info Header
        let GIHeader = document.createElement("h2");
        GIHeader.id = "GeneralInfoHeader";
        GIHeader.className = "StudentDataHeader";
        GIHeader.innerHTML = "General Info";
        StCard.appendChild(GIHeader);

        // General Info Table of rows
        let GInfo = document.createElement("div");
        GInfo.className = "GeneralInfo";

        // Last Name
        let lab = document.createElement("label");
        lab.innerHTML = "Last Name";
        GInfo.appendChild(lab);
        let p = document.createElement("p");
        p.innerHTML = student.LastName;
        GInfo.appendChild(p);

        // First Name
        lab = document.createElement("label");
        lab.innerHTML = "First Name";
        GInfo.appendChild(lab);
        p = document.createElement("p");
        p.innerHTML = student.FirstName;
        GInfo.appendChild(p);
      
        // Class - Level
        lab = document.createElement("label");
        lab.innerHTML = "Class - Level";
        GInfo.appendChild(lab);
        p = document.createElement("p");
        p.innerHTML = student.ClassLevel;
        GInfo.appendChild(p);

        StCard.appendChild(GInfo);
    }

    // Phones
    {
        // Phones Header
        let PHeader = document.createElement("h2");
        PHeader.id = "PhoneHeader";
        PHeader.className = "StudentDataHeader";
        PHeader.innerHTML = "Contact Phones";
        StCard.appendChild(PHeader);

        let Phones = document.createElement("div");
        Phones.className = "StudentPhones";
        

        // Phone
        let lab = document.createElement("label");
        lab.innerHTML = "Phone";
        Phones.appendChild(lab);
        let p = document.createElement("p");
        if (student.Phone) {
            p.innerHTML = student.Phone;
        }
        else {
            p.innerHTML = "-";
        }
        Phones.appendChild(p);


        // Mobile Phone
        lab = document.createElement("label");
        lab.innerHTML = "Mobile";
        Phones.appendChild(lab);
        p = document.createElement("p");
        if (student.Mobile) {
            p.innerHTML = student.Mobile
        }
        else {
            p.innerHTML = "-";
        }
        Phones.appendChild(p);


        // Other Phone 2
        lab = document.createElement("label");
        lab.innerHTML = "Other Phone 1";
        Phones.appendChild(lab);
        p = document.createElement("p");
        if (student.OtherPhone1) {
            p.innerHTML = student.OtherPhone1;
        }
        else {
            p.innerHTML = "-";
        }
        Phones.appendChild(p);


        // Other Phone 2
        lab = document.createElement("label");
        lab.innerHTML = "Other Phone 2";
        Phones.appendChild(lab);
        p = document.createElement("p");
        if (student.OtherPhone2) {
            p.innerHTML = student.OtherPhone2;
        }
        else {
            p.innerHTML = "-";
        }
        Phones.appendChild(p);

        
        StCard.appendChild(Phones);
    }

    // Schedules
    {
        let Schedules = document.createElement("div");
        Schedules.className = "StudentSchedules";

        const DayParts = ['Morning', 'Noon', 'Study'];

        // We need one table for each existing DayPart in student
        DayParts.forEach(function(dayPart) {
            let scheduleDiv = document.createElement("div");
            scheduleDiv.className = "ScheduleHeaderTable";

            // Schedule Header
            let Header = document.createElement("h2");
            Header.id = dayPart + "SchedulesHeader";
            Header.className = "StudentSchedulesHeader";
            Header.innerHTML = dayPart + " Schedules";
            scheduleDiv.appendChild(Header);

            let table = document.createElement("div");
            table.className = "Table";

            // Create the first row of headers
            let firstRow = document.createElement("div");
            firstRow.className = "SchedulesTableRow TableRow";
            
            let Headers = ['Schedule', 'Order', 'Time', 'Address', 'Note', 'Mon', 'Tue', 'Wen', 'Thu', 'Fri'];

            for (let i = 0; i < Headers.length; i++) {
                let p = document.createElement("p");
                p.className = "RowHeader";
                p.innerHTML = Headers[i];
                firstRow.appendChild(p);
            }

            table.appendChild(firstRow);

            let DayPartSchedules = dayPart + "Schedules";
            
            // If dayPart does not exist in students create a "p" with nothing in it
            if (student[DayPartSchedules].length === 0) {
                let p = document.createElement("p");
                p.className = "RowData";
                p.innerHTML = "-";
                scheduleDiv.appendChild(p);
            }


            for (i = 0; i < student[DayPartSchedules].length; i++) {
                let row = document.createElement("div");
                row.className = "SchedulesTableRow TableRow";

                let schedule = student[DayPartSchedules][i];
                // Bus Number
                p = document.createElement("p");
                p.className = "RowData";
                if (schedule.BusSchedule)
                    p.innerHTML = schedule.BusSchedule;
                else 
                    p.innerHTML = "-"
                row.appendChild(p)

                // Schedule Order (at what index student will be picked up)
                p = document.createElement("p");
                p.className = "RowData";
                if (schedule.ScheduleOrder)
                    p.innerHTML = schedule.ScheduleOrder;
                else 
                    p.innerHTML = "-"
                row.appendChild(p);

                p = document.createElement("p");
                p.className = "RowData";
                if (schedule.ScheduleTime)
                    p.innerHTML = schedule.ScheduleTime;
                else 
                    p.innerHTML = "-"
                row.appendChild(p);

                // Address
                p = document.createElement("p");
                p.className = "RowData";
                p.innerHTML = schedule.Address.FullAddress;
                row.appendChild(p);

                // Notes
                p = document.createElement("p");
                p.className = "RowData";
                if (schedule.Notes)
                    p.innerHTML = schedule.Notes;
                else
                    p.innerHTML = "-";
                row.appendChild(p);
                
                // Days
                let WeekDays = ['Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday'];

                for (let j = 0; j < WeekDays.length; j++) {
                    p = document.createElement("p");
                    p.className = "RowData";

                    if (schedule.Days[WeekDays[j]] === 1)
                        p.className += " OnDay";
                    else 
                        p.className += " OffDay";

                    row.appendChild(p);
                }

                table.appendChild(row);
                scheduleDiv.appendChild(table)
            }

            Schedules.appendChild(scheduleDiv);
            
        });  
        
        StCard.appendChild(Schedules);
    }

    MainInfo.appendChild(StCard)
}

function DisplayStudentMap(student) {
    // Display Student Map
    let studentsToPlot = [];

    // Find the id = key to Students "dict"
    let id = "\"" + student.ID + "\"";

    // *Subject to change* //
    // Add all addresses to one array, so it is easier to plot them all together.
    let Schedules = [];
    Schedules.push.apply(Schedules, student.MorningSchedules);
    Schedules.push.apply(Schedules, student.NoonSchedules);
    Schedules.push.apply(Schedules, student.StudySchedules);

    studentsToPlot.push({
        Name: student.LastName + ' ' + student.FirstName,
        Schedules: Schedules,
        Order: ""
    });

    return studentsToPlot;
}

    //#endregion //


    // #region Student Search               //

function OnMorePress() {
    let children = this.parentNode.childNodes;
    let id = children[0].innerHTML;
    id = "\"" + id + "\"";

    let student;
    let CurrentStudents = SearchTabGroup.activeTab().students;

    // If students exist in active tab's students get it from those.
    if (CurrentStudents.hasOwnProperty(id)) {
        student = CurrentStudents[id];

        let title = student.LastName + " " + student.FirstName;
        let type = "StudentCard"
        let newSearchTab = OpenSearchTab(docmain, title, type, DisplayStudentCard, DisplayStudentMap, student);
        newSearchTab.activate(false);
    }
    // else get a new one
    else {
        id = children[0].innerHTML;

        let sql = "Select * From Student, Address, Schedule\
        Where Student.AddressID = Address.AddressID and Schedule.StudentID = Student.StudentID and\
        Student.StudentID = \"" + id + "\"";

        id = "\"" + id + "\"";
        
        searchobj = ExecuteSQLToProc(sql, function() {
            CurrentStudents  = StudentJsonRead(searchobj.json_file)
            let student = CurrentStudents[id]

            let title = student.LastName + " " + student.FirstName;
            let type = "StudentCard"
            let newSearchTab = OpenSearchTab(docmain, title, type, DisplayStudentCard, DisplayStudentMap, student);
            newSearchTab.activate(false);
        });
    }

    //OpenBottomBar();
}

function SearchStudents() {
    // Filters handler and Tab Creator
    // *SQL Injection* //

    // Grab all filters from search bars
    const FirstName = document.getElementById("FirstNameBar").value;
    const LastName = document.getElementById("LastNameBar").value;
    const Class = document.getElementById("ClassBar").value;
    const Level = document.getElementById("LevelBar").value;
    const Street = document.getElementById("StreetBar").value;
    const Number = document.getElementById("NumberBar").value;
    const Municipal = document.getElementById("MunicipalBar").value;
    const ZipCode = document.getElementById("ZipCodeBar").value;
    const DayPart = document.getElementById("DayPartBar").value;

    const SearchValues = [FirstName, LastName, Class, Level, DayPart, Street, Number, Municipal, ZipCode];
    const SearchFields = ["Student.FirstName", "Student.LastName", "Student.Class", "Student.Level", "Schedule.DayPart",
     "studAd.Road", "studAd.Number", "studAd.Municipal", "studAd.ZipCode"];

    let toSearch = "Where Schedule.AddressID = schedAd.AddressID and Student.AddressID = studAd.AddressID and Student.StudentID = Schedule.StudentID"

    // Check if no filters are given.
    let empty = true;
    SearchValues.forEach(function(value) {
        if (value) {
            empty = false;
        }
    });

    if (empty) {
        if(!confirm("You have given no filters. This will display the whole database. Are you sure?"))
            return;
    }

    // Create SQL query
    for (let i = 0; i < SearchValues.length; i++) {
        if (SearchValues[i]) {
            SearchValues[i] = SearchValues[i].toUpperCase();
            toSearch += (" and " + SearchFields[i] + " Like \"%" + SearchValues[i] + "%\"");
        }
    }

    DayParts = ['Morning', 'Noon', 'Study'];
    
    let loading = document.getElementById("StudentSearchButton").childNodes[1];
    loading.hidden = false;

    loading.nextElementSibling.innerHTML = "Searching"

    let sql = "Select *\
            From Student, Address as studAd, Address as schedAd, Schedule " + toSearch + " Order By Student.LastName";
    

    // Execute query and get Students
    let searchobj = ExecuteSQLToProc(sql, function() {
        let title;

        if (empty) {
            title = "All Students";
        }
        else {
            // Student Search : 
            title = "\"" + FirstName + " " + LastName + " " + Class + " " + Level + " " + DayPart + " " +
             Street + " " + Number + " " + Municipal + " " + ZipCode + "\"";
        }

        let newSearchTab
        if (!DayPart) {
            let Students = StudentJsonRead(searchobj.json_file)
        
            // Open a new SearchTab, which displays Students as a Table and Map Tabs.
            let type = "Student"
            newSearchTab = OpenSearchTab(docmain, title, type, DisplayStudentSearchTable, DisplayStudentSearchMap, Students);
        }
        else {
            let Students = ScheduleJsonRead(searchobj.json_file)
            let type = "Schedule"
            newSearchTab = OpenSearchTab(docmain, title, type, DisplayBusTable, DisplayBusMap, Students);
        }

        newSearchTab.activate(false);

        // Assign onclick to More Buttons.
        ReassignAllButtons(); 

        loading.hidden = true;
        loading.nextElementSibling.innerHTML = "Search Student(s)"
    });

}

    // #endregion //


// #endregion                   //



// #region Search Bars Handlers //


    // Clear search bars click handler //
function ClearSearchBars() {
    document.getElementById("FirstNameBar").value = "";
    document.getElementById("LastNameBar").value = "";
    document.getElementById("ClassBar").value = "";
    document.getElementById("LevelBar").value = "";
    document.getElementById("StreetBar").value = "";
    document.getElementById("NumberBar").value = "";
    document.getElementById("MunicipalBar").value = "";
    document.getElementById("ZipCodeBar").value = "";
    document.getElementById("DayPartBar").value = "";
}

    // Onclick assignment //
function OnSearchClearStudent() {   
    // Assign OnClick functions to search-clear buttons.    //
    let StudentSearchButton = document.getElementById("StudentSearchButton");
    StudentSearchButton.onclick = SearchStudents;

    let StudentClearButton = document.getElementById("StudentClearButton");
    StudentClearButton.onclick = ClearSearchBars;
}

function OnEnterSearchStudents(event) {
    if (!event) event = window.event;
    if (event.key === "Enter") {
        SearchStudents();
    }
}

function KeyDownSearchBars() {
    let bars = document.getElementsByClassName("SearchBar");
    for (let i = 0; i < bars.length; i++) {
        bars[i].onkeydown = OnEnterSearchStudents;
    }
}

// #endregion //



// #region Search Tabs Handlers //


function OpenSearchTab(element, title, type, infoDisplayFunction, mapDisplayFunction, students) {
    // Create a new Search Tab and Display it.  //
    let newTab = new Tab([element], title, type, true);
    SearchTabGroup.addTab(newTab, OnSearchTabPress, OnCloseTabPress);
    newTab.students = students;

    // Depending on the process we want to do, we give different functions  //
    // for info and map display.    //
    newTab.infoDisplayFunction = infoDisplayFunction;
    newTab.mapDisplayFunction = mapDisplayFunction;

    DisplaySearchTab(newTab);

    CheckDisabledScheduleButton(newTab);

    return newTab;
}

function DisplaySearchTab(tab) {
    MainInfo.innerHTML = "";
    InfoMapTabHeader.innerHTML = "";

    let prevActive = null;

    if (tab.subTabGroup) {
        prevActive = tab.subTabGroup.currentActive;
    }

    // Create a subTabGroup in tab to hold Info and Map //
    let InfoMapTabGroup = new TabGroup(InfoMapTabHeader);
    tab.subTabGroup = InfoMapTabGroup;

    // Display the info and store it to a new InfoTab   //
    tab.infoDisplayFunction(tab.students);
    let InfoTab = new Tab([MainInfo], "Info", "Info", false);
    InfoMapTabGroup.addTab(InfoTab, OnInfoTabPress);

    // Reset html to create a new MapTab    //
    MainInfo.innerHTML = "";
    // Use mapDisplayFunction to get all the addresses we want to plot  //
    tab.studentsToPlot = tab.mapDisplayFunction(tab.students);

    // Create a new MapTab to hold the map  //
    // Note: This does not actually hold the map. Rather, it will render the map when it is pressed.    //

    let MapTab = new Tab([MainInfo], "Map", "Map", false);
    InfoMapTabGroup.addTab(MapTab, OnMapTabPress);

    // Keep history of which tab was active when user changed SearchTabs    //
    if (prevActive === 0) {
        InfoTab.activate();
    }
    else if (prevActive === 1) {
        MapTab.activate();
        MainInfo.innerHTML = "";

        CreateMap(tab);
        PlotStudents(tab);
    }
    else if (prevActive === null) {
        InfoTab.activate();
    }
    
    tab.updateContents();

    if (tab.type === "StudentCard") OpenBottomBar();
    else CloseBottomBar();
}

// Click Handlers
function OnSearchTabPress(event) {
    // When search Tab is pressed display it and activate it.   //
    // Note: Displaying here is important because buttons(onclicks) and maps cannot be stored in any other way. //
    let pressedTab = SearchTabGroup.getPressed(this);
    if (pressedTab === SearchTabGroup.activeTab() || pressedTab.closed || closing) {
        closing = false;
        return;
    }

    DisplaySearchTab(pressedTab);
    pressedTab.activate(false);

    // if (pressedTab.type !== "StudentCard") {
    //     CloseBottomBar();
    // }
    // else {
    //     OpenBottomBar();
    // }

    CheckDisabledScheduleButton(pressedTab)

    ReassignAllButtons();
}

function OnClearTabsPress() {
    SearchTabGroup.clearTabs();

    CacheDOM();
    InfoMapTabHeader.innerHTML = "";
    MainInfo.innerHTML = "";

    CheckDisabledScheduleButton(null);
    CloseBottomBar();
}

function OnCloseTabPress() {
    closing = true;
    let a = SearchTabGroup.closePressed(this);
    if (SearchTabGroup.length === 0) {
        InfoMapTabHeader.innerHTML = "";
        MainInfo.innerHTML = "";
    }
    else {
        DisplaySearchTab(SearchTabGroup.activeTab());
        SearchTabGroup.activeTab().activate(false);
        ReassignAllButtons();
    }
    CheckDisabledScheduleButton(SearchTabGroup.activeTab())
}

function CloseCurrentTab() {
    let active = SearchTabGroup.activeTab();

    active.close();

    if (SearchTabGroup.length === 0) {
        InfoMapTabHeader.innerHTML = "";
        MainInfo.innerHTML = "";
    }
    else {
        DisplaySearchTab(SearchTabGroup.activeTab());
        SearchTabGroup.activeTab().activate(false); 
        ReassignAllButtons();
    }
    CheckDisabledScheduleButton(SearchTabGroup.activeTab())
}

// "Info - Map" Click handlers

function OnInfoTabPress() {
    SearchTabGroup.activeTab().subTabGroup.activatePressed(this);
    ReassignAllButtons();
}

function OnMapTabPress() {
    // Clears MainInfo and creates a new map with markers.
    let searchTab = SearchTabGroup.activeTab();
    searchTab.subTabGroup.activatePressed(this);
    MainInfo.innerHTML = "";
    
    CreateMap(searchTab);
    PlotStudents(searchTab);
}

// #endregion //



// #region Map Functions        //

function CreateMap(tab) {
    let Students = tab.studentsToPlot;
    // Create an empty map: //
    let coords = [];

    for (let i = 0; i < Students.length; i++) {
        let student = Students[i];
        for (let j = 0; j < student.Schedules.length; j++) {
            let address = student.Schedules[j].Address;
            coords.push([address.Latitude, address.Longitude]);
        }
    }

    // Find a "centroid" from all addresses given so map can center there.  //
    let Ox = 0;
    let Oy = 0;
    coords.forEach(element => {
        Ox += element[0];
        Oy += element[1];
    });

    Ox /= coords.length;
    Oy /= coords.length;
    
    // map is a global variable which hold the CURRENT open map.            //
    let Map = document.createElement("div");
    Map.id = "Map";
    Map.className = "Map";
    MainInfo.appendChild(Map);
    map = khtml.maplib.Map(document.getElementsByClassName("Map")[0]);
    map.centerAndZoom(new khtml.maplib.LatLng(Ox, Oy), 12);
}

function PlotStudents(tab) {
    let Students = tab.studentsToPlot;
    if (tab.hasOwnProperty("activeBuses")) {
        if (tab.activeBuses.length > 1) {
            PlotSchedules(Students, tab.activeBuses);
            return;
        }
    }

    // Define icon to render
    let defaultIcon = {
		url: "../images/Markers/red-dot.png",
		size: {width: 26, height: 32},
		origin: {x: 0, y: 0},
		anchor: {
			x: "-16px",
			y: "-32px"
		}
	};
    

    let plottedAddresses = {};
    for (let i = 0; i < Students.length; i++) {
        let student = Students[i];
        let studentName = student.Name;

        for (let j = 0; j < student.Schedules.length; j++) {
            let address = student.Schedules[j].Address;
            let schedule = student.Schedules[j].BusSchedule;
            let time = student.Schedules[j].ScheduleTime;
            let key = address.Latitude + "," + address.Longitude;
            let found = false;

            // If an address already has been saved, dont re-save it    //
            // Add it's StudentName to the title instead                //
            if (plottedAddresses.hasOwnProperty(key)) {
                let title = "";

                let index = 1;
                plottedAddresses[key].names.forEach(name => {
                    if (name === studentName) {
                        found = true;
                    }
                    title += (index + ") " + name + "\n");
                    index++;
                });
                if (found) continue;
                
                // InfoBox- Format: //
                // 1) John Smith    //
                // 2) George Black  //
                // ...              //
                // <ScheduleName>   //
                // <PickupTime>     //
                title += (index + ") " + studentName + "\n");
                title += address.FullAddress + "\n";
                title += "Schedule: " + schedule + "\n";
                title += "Time: " + time;
                plottedAddresses[key].names.push(studentName)
                plottedAddresses[key].title = title;              

            }
            else {
                plottedAddresses[key] = {
                    title: "1) " + studentName + "\n" + address.FullAddress + "\n" + "Schedule: " + schedule + "\n" + "Time: " + time,
                    names: [studentName],
                    address: address,
                    schedule: schedule,
                    order: student.Order,
                    time: time
                };
            }
        }
    }

    // Iterate through all addresses and create markers on their coords     //edAddressKeys = Object.keys(plottedAddresses);
    let plottedAddressKeys = Object.keys(plottedAddresses);
    for (let i = 0; i < plottedAddressKeys.length; i++) {
        let toMark = plottedAddresses[plottedAddressKeys[i]];

        let icon
        if (toMark.order === "")
            icon = defaultIcon;
        else {
            icon = {
                url: MarkerURL + toMark.order + MarkerColors[0],
                size: {width: 26, height: 32},
                origin: {x: 0, y: 0},
                anchor: {
                    x: "-16px",
                    y: "-32px"
                }
            }
        }


        let marker = new khtml.maplib.overlay.Marker({
            position: new khtml.maplib.LatLng(toMark.address.Latitude, toMark.address.Longitude), 
            map: map,
            title: toMark.title,
            names: toMark.names,
            address: toMark.address,
            schedule: toMark.schedule,
            icon: icon
        });
    }
}

function PlotSchedules(Students, Schedules) {

    SchedulesIcons = {};
    let tooMany = false;

    // If there are too many schedules (> 5), marker label will be scheduleName, not order inside schedule  //
    if (Schedules.length > 5) {
        tooMany = true;
    }
    for (let i = 0; i < Schedules.length; i++) {
        SchedulesIcons[Schedules[i]] = {
            Schedule: Schedules[i],
            MarkerIndex: i,
        }
    }

    // For each student add a marker to the current open map, depending on student's schedule
    let plottedAddresses = {};
    for (let i = 0; i < Students.length; i++) {
        let student = Students[i];
        let studentName = student.Name;

        for (let j = 0; j < student.Schedules.length; j++) {
            let address = student.Schedules[j].Address;
            let schedule = student.Schedules[j].BusSchedule;
            let key = address.Latitude + "," + address.Longitude;
            let found = false;

            // If an address already has been saved, dont re-save it    //
            // Add it's StudentName to the title instead                //
            if (plottedAddresses.hasOwnProperty(key)) {
                let title = "";

                let index = 1;
                plottedAddresses[key].names.forEach(name => {
                    if (name === studentName) {
                        found = true;
                    }
                    title += (index + ") " + name + "\n");
                    index++;
                });
                if (found) continue;
                
                // InfoBox- Format: //
                // 1) John Smith    //
                // 2) George Black  //
                // ...              //
                // <ScheduleName>   //
                // <PickupTime>     //
                title += (index + ") " + studentName + "\n");
                title += address.FullAddress + "\n";
                title += schedule;
                plottedAddresses[key].names.push(studentName)
                plottedAddresses[key].title = title;              

            }
            else {
                plottedAddresses[key] = {
                    title: "1) " + studentName + "\n" + address.FullAddress + "\n" + schedule + "\n" + student.Schedules[j].Time,
                    names: [studentName],
                    address: address,
                    schedule: schedule,
                    order: student.Order,
                    time: student.Schedules[j].Time
                };
            }
        }
    }

    // Iterate through all addresses and create markers on their coords     //
    let plottedAddressKeys = Object.keys(plottedAddresses);
    for (let i = 0; i < plottedAddressKeys.length; i++) {
        let toMark = plottedAddresses[plottedAddressKeys[i]];

        let url;

        if (tooMany) {
            url = MarkerURL
            if (toMark.schedule.length > 1) {
                url += (toMark.schedule[0] + toMark.schedule[1]);
            }
            else {
                url += toMark.schedule[0];
            }
            url +=MarkerColors[SchedulesIcons[toMark.schedule].MarkerIndex];
        }
        else {
            url = MarkerURL + toMark.order + MarkerColors[SchedulesIcons[toMark.schedule].MarkerIndex];
        }

        let marker = new khtml.maplib.overlay.Marker({
            position: new khtml.maplib.LatLng(toMark.address.Latitude, toMark.address.Longitude), 
            map: map,
            title: toMark.title,
            names: toMark.names,
            address: toMark.address,
            schedule: toMark.schedule,
            icon: {
                // Give different-colored Markers foreach different schedule    //
                url: url,
                size: {width: 26, height: 32},
                origin: {x: 0, y: 0},
                anchor: {
                    x: "-16px",
                    y: "-32px"
                }
            }
        });
    }
}

// #endregion //



// #region DB Retrieval-Parsing //

// Executes sql in a python process and handles data returned in callback parameter //
function ExecuteSQLToProc(sql, callback) {
    let toJson = {
        Database: DBFile,
        sql: sql
    }

    // Create a json request file to hold the query and DB file //
    fs.writeFile(datadir + "tmp/sql.json", JSON.stringify(toJson), (err) => {
        if (err) {
            alert(err)
            console.error(err);
            return;
        };
    })

    // Spawn process to run sql query   //
    let proc = spawn('python', [pythondir + "SQLHandler.py", datadir + "tmp/sql.json"])

    proc.stderr.on('data', function(data) {
        alert(data.toString());
        console.error(data.toString());
    });

    // Call callback function when process closes   //
    proc.on('close', callback);
    
    return {
        process: proc,
        json_file: datadir + "/tmp/sql.json"
    }
}

function StudentJsonRead(json_file) {

    let json_content;
    let raw_data = fs.readFileSync(json_file);
    let raw_students = JSON.parse(raw_data)
    let Rows = raw_students.Rows;

    let Students = {};

    for (let i = 0; i < Rows.length; i++) {
        let row = Rows[i];

        let id = "\"" + row.StudentID + "\"";
        // If student has not already been saved save it    //
        if (!Students.hasOwnProperty(id)) {
            // ClassLevel: "Β - ΔΗΜΟΤΙΚΟΥ" or "ΝΗΠΙΑΓΩΓΕΙΟ" //
            let classLevel;
            if (!row.Class) 
                classLevel = row.Level;
            else
                classLevel = row.Class + " - " + row.Level + "Y";

            student = {
                ID: row.StudentID,
                Name: row.LastName + " " + row.FirstName,
                ClassLevel: classLevel,
                FirstName: row.FirstName,
                LastName: row.LastName,

                Phone: row.Phone,
                Mobile: row.Mobile,
                OtherPhone1: row.OtherPhone1,
                OtherPhone2: row.OtherPhone2,

                MorningSchedules: [],

                NoonSchedules: [],

                StudySchedules: [],

            };
            
            student[row.DayPart + "Schedules"].push({
                ScheduleID: row.ScheduleID,
                Address: {
                    FullAddress: row.FullAddress,
                    Road: row.Road,
                    Number: row.Number,
                    ZipCode: row.ZipCode,
                    Municipal: row.Municipal,
                    AddressID: row.AddressID,
                    Longitude: row.GPS_X,
                    Latitude: row.GPS_Y
                },
                Days: {
                    Monday: row.Monday,
                    Tuesday: row.Tuesday,
                    Wednesday: row.Wednesday,
                    Thursday: row.Thursday,
                    Friday: row.Friday
                },
                Notes: row.FullNote,
                BusSchedule: row.BusSchedule,
                ScheduleOrder: row.ScheduleOrder,
                ScheduleTime: row.ScheduleTime,
                Early: row.Early,
                Late: row.Late,
                Around: row.Around,
                DayPart: row.DayPart
            });

            Students[id] = student;
        }
        else {
            // Ιf student exists save only its DIFFERENT addresses, days, notes, buses PER daypart.

            Students[id][row.DayPart + "Schedules"].push({
                ScheduleID: row.ScheduleID,
                Address: {
                    FullAddress: row.FullAddress,
                    AddressID: row.AddressID,
                    Road: row.Road,
                    Number: row.Number,
                    ZipCode: row.ZipCode,
                    Municipal: row.Municipal,
                    Longitude: row.GPS_X,
                    Latitude: row.GPS_Y
                },
                Days: {
                    Monday: row.Monday,
                    Tuesday: row.Tuesday,
                    Wednesday: row.Wednesday,
                    Thursday: row.Thursday,
                    Friday: row.Friday
                },
                Notes: row.FullNote,
                BusSchedule: row.BusSchedule,
                ScheduleOrder: row.ScheduleOrder,
                ScheduleTime: row.ScheduleTime,
                Early: row.Early,
                Late: row.Late,
                Around: row.Around,
                DayPart: row.DayPart
            });

        }
    }

    fs.unlink(json_file, (err) => {
        if (err) {
            alert(err);
            console.error(err)
        }
    })

    return Students;
}

function ScheduleJsonRead(json_file) {
    let json_content;
    let raw_data = fs.readFileSync(json_file);
    let raw_students = JSON.parse(raw_data)
    let Rows = raw_students.Rows;

    let Students = [];

    for (let i = 0; i < Rows.length; i++) {
        let row = Rows[i];

        let id = "\"" + row.StudentID + "\"";

        let classLevel;
        if (!row.Class)
            classLevel = row.Level;
        else 
            classLevel = row.Class + " - " + row.Level + "Y";

        student = {
            ID: row.StudentID,
            Name: row.LastName + " " + row.FirstName,
            ClassLevel: classLevel,
            FirstName: row.FirstName,
            LastName: row.LastName,

            Phone: row.Phone,
            Mobile: row.Mobile,
            OtherPhone1: row.OtherPhone1,
            OtherPhone2: row.OtherPhone2,

            DayPart: row.DayPart,
            ScheduleID: row.ScheduleID,

            Address: {
                FullAddress: row.FullAddress,
                Road: row.Road,
                Number: row.Number,
                ZipCode: row.ZipCode,
                Municipal: row.Municipal,
                AddressID: row.AddressID,
                Longitude: row.GPS_X,
                Latitude: row.GPS_Y
            },

            Days: {
                Monday: row.Monday,
                Tuesday: row.Tuesday,
                Wednesday: row.Wednesday,
                Thursday: row.Thursday,
                Friday: row.Friday
            },

            Notes: row.FullNote,
            BusSchedule: row.BusSchedule,
            ScheduleTime: row.ScheduleTime,
            ScheduleOrder: row.ScheduleOrder,
            Early: row.Early,
            Late: row.Late,
            Around: row.Around,
            DayPart: row.DayPart
        }

        Students.push(student);
    }

    fs.unlink(json_file, (err) => {
        if (err) {
            alert(err);
            console.error(err)
        }
    })

    return Students;
}

// #endregion



// #region General Utility      //

function ReassignAllButtons() {
    let moreButtons = document.getElementsByClassName("MoreButton");

    for (let i = 0; i < moreButtons.length; i++) {
        moreButtons[i].onclick = OnMorePress;
    }

    // Assign OnClick functions to search-clear buttons.    //
    let StudentSearchButton = document.getElementById("StudentSearchButton");
    StudentSearchButton.onclick = SearchStudents;

    let StudentClearButton = document.getElementById("StudentClearButton");
    StudentClearButton.onclick = ClearSearchBars;

    OnBusClickHandle();
}

function CacheDOM() {
    // Caching DOM elements.    //
    docmain = document.getElementsByTagName("main")[0];
    SearchTabHeader = document.getElementsByClassName("SearchTabGroup")[0];
    InfoMapTabHeader = document.getElementsByClassName("InfoMapTabGroup")[0];
    MainInfo = document.getElementsByClassName("MainInfo")[0];
}
// #endregion //


// #region Navigation Bars

function OpenSideBar() {
    this.onclick = CloseSideBar
    

    let sidebar = document.getElementsByClassName("SideBar")[0]
    
    sidebar.style.width = "20vw";
    document.getElementById("BottomNavBar").style.width = "calc(80vw - 30px)";
    document.getElementsByTagName("body")[0].style.marginLeft = "calc(20vw + 30px)";

    setTimeout(() => {
        
        for (let i = 0; i < sidebar.childElementCount; i++) {
            sidebar.children[i].style.opacity = "1";
        }
        this.children[0].src = "../images/General/hide.png"
        sidebar.style.overflowY = "scroll";

        let prevActive = null;

        let tab = SearchTabGroup.activeTab();
        if (tab) {
            if (tab.subTabGroup) {
                prevActive = tab.subTabGroup.currentActive;
            }
            if (prevActive === 1) {
                MainInfo.innerHTML = "";
        
                CreateMap(tab);
                PlotStudents(tab);
            }
        }
        
    },  410);
}

function CloseSideBar() {
    this.onclick = OpenSideBar;
    

    let sidebar = document.getElementsByClassName("SideBar")[0]
    
    for (let i = 0; i < sidebar.childElementCount; i++) {
        sidebar.children[i].style.opacity = "0";
    }
    
    setTimeout(() => {
        sidebar.style.overflowY = "hidden";
        sidebar.style.width = "0";
        document.getElementsByTagName("body")[0].style.marginLeft = "30px";
        document.getElementById("BottomNavBar").style.width = "100vw";
        this.children[0].src = "../images/General/show.png"

        setTimeout(() => {
            let prevActive = null;

            let tab = SearchTabGroup.activeTab();
            if (tab) {
                if (tab.subTabGroup) {
                    prevActive = tab.subTabGroup.currentActive;
                }
                if (prevActive === 1) {
                    MainInfo.innerHTML = "";
            
                    CreateMap(tab);
                    PlotStudents(tab);
                }
            }
        }, 400);
        
    }, 400);
}

function OpenBottomBar() {
    document.getElementById("BottomNavBar").style.height = "50px";
}

function CloseBottomBar() {
    document.getElementById("BottomNavBar").style.height = "0";
}

// #endregion



// #region Editors-Printer-Loader & Event Handlers       //

    // Printer
function PrintHandler() {
    // Sends message to main process to print MainInfo. //
    document.getElementById("PrintButton").addEventListener("click", () => {
        let title = SearchTabGroup.activeTab().title;
        let type = SearchTabGroup.activeTab().type;
        ipcRenderer.send("Print", MainInfo.innerHTML, title, type);
    });
}

function StudentEditorHandler() {
    document.getElementById("EditButton").addEventListener("click", () => {
        let Student = SearchTabGroup.activeTab().students;
        ipcRenderer.send("OpenStudentEditor", Student);
    });
}


    // Loader
function OnCreateWindow() {
    DBFile = datadir + "MMGP_data.db";
    spawn = require('child_process').spawn;
    fs = require('fs');

    let raw_data = fs.readFileSync(__dirname + "/../images/colors.json");
    let data = JSON.parse(raw_data)

    MarkerColors = data.colors;
    MarkerURL = data.url;

    // ipcRenderer = require("electron").ipcRenderer;
    
    // GenerateBusButtons();
    GenerateBusDayPartButtons();
    OnSearchClearStudent();
    KeyDownSearchBars();
    PrintHandler();
    StudentEditorHandler();
    CacheDOM();

    // Create a SearchTabGroup and hold it to a global variable for use.    //
    SearchTabGroup = new TabGroup(document.getElementsByClassName("SearchTabGroup")[0])

    document.getElementById("ClearTabsButton").onclick = OnClearTabsPress;

    ReassignAllButtons();

    document.getElementById("Show\/HideButton").onclick = CloseSideBar;
}

ipcRenderer.on("CloseTab", (event) => {
    CloseCurrentTab();
})

ipcRenderer.on("CloseAllTabs", (event) => {
    OnClearTabsPress();
})

ipcRenderer.on("Print", (event) => {
    let title = SearchTabGroup.activeTab().title;
    let type = SearchTabGroup.activeTab().type;
    ipcRenderer.send("printPDF", MainInfo.innerHTML, title, type);
})

ipcRenderer.on("Edit", (event) => {
    if (SearchTabGroup.activeTab().type !== "StudentCard") return;
    
    let Student = SearchTabGroup.activeTab().students;
    ipcRenderer.send("OpenStudentEditor", Student);
})

// #endregion //