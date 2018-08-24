// External modules
let sqlite3;
let db;
let DOMElementHistory;
let spawn;

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

// Marker-Images
let Markers = ["../images/Markers/blue-dot.png", "../images/Markers/red-dot.png", 
"../images/Markers/green-dot.png", "../images/Markers/yellow-dot.png",
"../images/Markers/purple-dot.png", "../images/Markers/orange-dot.png", "../images/Markers/pink-dot.png"];

let pythondir = __dirname + "/../../python/"
let datadir = __dirname + "/../../data/"

// Bus Searching

function GetActiveDayPart() {
    var buttons = document.getElementsByClassName("DayPartSelectorButton");
    var active = null;

    for (let i = 0; i < buttons.length; i++) {
        if (buttons[i].classList.contains("active")) {
            active = buttons[i];
            break;
        }
    }

    return active;
}

function GetActiveSchedule() {
    var DropDown = document.getElementById("ScheduleSelectorDropDown");
    return DropDown.value;
}

function GetActiveBus() {
    var buttons = document.getElementsByClassName("BusButton");
    var active = null;

    for (let i = 0; i < buttons.length; i++) {
        if (buttons[i].classList.contains("active")) {
            active = buttons[i];
            break;
        }
    }

    return active
}

function OnBusClickHandle() {
    var buttons = document.getElementsByClassName("DayPartSelectorButton");
    
    for (let i = 0; i < buttons.length; i++) {
        buttons[i].onclick = OnDayPartClick;
    }

    var busbuttons = document.getElementsByClassName("BusButton");
    

    setTimeout(() => {
        for (let i = 0; i < busbuttons.length; i++) {
            busbuttons[i].onclick = OnBusClick;
        }
    }, 20);
    

    var button = document.getElementById("ScheduleSearchButton");
    button.onmouseup = SearchSchedule;

    button = document.getElementById("AddScheduleButton");
    button.onmouseup = AddSchedule;

    button = document.getElementById("CalculateDurationButton");
    button.onclick = CalculateScheduleDuration;

}

function OnDayPartClick() {
    var prevActive = GetActiveDayPart();
    if (prevActive)
        prevActive.classList.remove("active");
    this.classList.add("active");
    GenerateScheduleButtons();
}

function OnScheduleClick() {
    var prevActive = GetActiveSchedule();
    if (prevActive)
        prevActive.classList.remove("active");
    this.classList.add("active");
}

function OnBusClick() {
    var prevActive = GetActiveBus();
    if (prevActive)
        prevActive.classList.remove("active");
    this.classList.add("active");
}

function GetScheduleSearchCriteria() {
    let activeBus = null;
    let activeBusButton = GetActiveBus();

    if (activeBusButton)
        activeBus = activeBusButton.innerHTML

    let activeDayPartButton = GetActiveDayPart()
    let activeSchedule = GetActiveSchedule();

    if (!activeDayPartButton || !activeSchedule) {
        alert("Error: No DayPart or Schedule given.");
        return;
    }

    let activeDayPart = activeDayPartButton.innerHTML

    let busSchedule;

    switch(activeDayPart) {
        case "Morning":
            if (!activeBus) {
                alert("Error: No Bus given.");
                return
            }
            busSchedule = activeBus + "Π" + activeSchedule;
            break;
        case "Noon":
            if (!activeBus) {
                alert("Error: No Bus given.");
                return
            }
            busSchedule = activeBus + "Μ" + activeSchedule;
            break;
        case "Study":
            busSchedule = activeSchedule;
            break;
        default: break;
    }

    return busSchedule;
}

function SearchSchedule() {
    let busSchedule = GetScheduleSearchCriteria();

    let sql = "Select * From Student, Address Where Student.AddressID = Address.AddressID and BusSchedule = \"" + busSchedule + "\" Order By ScheduleOrder";
    let Students = GetBusFromDB(sql);

    let title = busSchedule;
    let waitTime = 20;
    let type = "Schedule";

    setTimeout(() => {     
        let newSearchTab = OpenSearchTab(docmain, title, type, DisplayBusTable, DisplayBusMap, Students);
        newSearchTab.activate(false);
        newSearchTab.activeBuses = [busSchedule];

        // Assign onclick to More Buttons.
        ReassignAllButtons(); 
        CurrentStudents = Students;
    }, waitTime);
}
 
function CalculateScheduleDuration() {
    let activeTab = SearchTabGroup.activeTab();
    let Students = activeTab.students;

    let loading = document.getElementById("CalculateDurationButton").childNodes[1];
    loading.hidden = false;

    loading.nextElementSibling.innerHTML = "Calculating"

    let toJson = {
        students: []
    }

    for (let i = 0; i < Students.length - 1; i++) {
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


    var fs = require("fs");

    fs.writeFile(datadir + "tmpsched.json", JSON.stringify(toJson), (err) => {
        if (err) {
            console.error(err);
            return;
        };
    })
    const spawn = require("child_process").spawn;
    var proc = spawn('python', [pythondir + "CalculateScheduleDuration.py", datadir + "tmpsched.json"]);

    proc.on('close', function(code) {
        console.log("Calculating Completed");
        fs.unlink(datadir + "tmpsched.json", (err) => {
            if (err)
                console.error(err)
        })
    })

    proc.stdout.on('data', function(data) {
        console.log(data.toString());
        data = data.toString().split('\n');
        let Distance = data[0];
        let Duration = data[1];

        let Minutes = Math.ceil(Duration / 60)
        let Seconds = Math.ceil(Duration % 60)

        alert("Distance: " + (Distance / 1000).toString() + "km\nDuration: " + Minutes.toString() + "min " + Seconds.toString() + "sec.");
        loading.hidden = true;

        loading.nextElementSibling.innerHTML = "Calculate Duration"
    })

    proc.stderr.on('data', function(data) {
        console.error(data.toString());
    }) 

}

function GetDistanceFromDB(sql) {
    let Distances = [];

    db.each(sql, function(err,row) {
        Distances.push({
            AddressID_1: row.AddressID_1,
            AddressID_2: row.AddressID_2,
            Distance: row.Distance,
            Duration: row.Duration
        })
    })

    return Distances
}

function GetDepotFromDB() {
    let sql = "Select * From Depot"
    let Depot = {}

    db.each(sql, function(err, row) {
        Depot = {
            AddressID: row.AddressID,
            FullAddress: row.FullAddress,
            Longitude: row.GPS_X,
            Latitude: row.GPS_Y
        }
    })

    return Depot
}

function AddSchedule() {
    let activeTab = SearchTabGroup.activeTab()

    if (activeTab.type !== "Schedule") {
        return;
    }
    
    let busSchedule = GetScheduleSearchCriteria();

    if (activeTab.hasOwnProperty("activeBuses")) {
        if (activeTab.activeBuses.length >= Markers.length) {
            alert("Can't have more than 7 open schedules at once.");
            return;
        }
        if (activeTab.activeBuses.includes(busSchedule)) {
            alert("This Schedule is already open in this tab.");
            return;
        }
    }

    let sql = "Select * From Student, Address Where Student.AddressID = Address.AddressID and BusSchedule = \"" + busSchedule + "\" Order By ScheduleOrder";
    let Students = GetBusFromDB(sql);


    let title = activeTab.title + ", " + busSchedule;
    let type = "Schedule";
    let waitTime = 20;

    setTimeout(() => {
        Students.push.apply(Students, activeTab.students)
        let newSearchTab = OpenSearchTab(docmain, title, type, DisplayBusTable, DisplayBusMap, Students);

        newSearchTab.activeBuses = activeTab.activeBuses
        newSearchTab.activeBuses.push(busSchedule);
        

        newSearchTab.activate(false);
        activeTab.close();
        
        // Assign onclick to More Buttons.
        ReassignAllButtons(); 
        CurrentStudents = Students;
    }, waitTime);
}

function DisplayBusTable(Students) {
    var table = document.createElement("div")
    table.className = "Table"


    // Create the first row of headers
    var firstRow = document.createElement("div")
    firstRow.className = "BusTableRow TableRow"

    var Headers = [' ', 'Bus Schedule', 'Schedule Order', 'Last Name', 'First Name', 'Class - Level', 'Address', 'Mon', 'Tue', 'Wen', 'Thu', 'Fri']

    for (var i = 0; i < Headers.length; i++) {
        var p = document.createElement("p")
        p.className = "RowHeader"
        p.innerHTML = Headers[i]
        firstRow.appendChild(p)
    }

    table.appendChild(firstRow);

    for (var i = 0; i < Students.length; i++) {
        let student = Students[i];
        var row = document.createElement("div")
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
        let searchimg = document.createElement("img");
        searchimg.src = "../images/General/more.png";
        searchimg.className = "MoreButtonImage"
        button.appendChild(searchimg);
        row.appendChild(button);

        p = document.createElement("p")
        p.className = "RowData"
        p.innerHTML = student.BusSchedule;
        row.appendChild(p)

        p = document.createElement("p")
        p.className = "RowData"
        p.innerHTML = student.ScheduleOrder;
        row.appendChild(p)
        
        p = document.createElement("p")
        p.className = "RowData"
        p.innerHTML = student.LastName
        row.appendChild(p)

        p = document.createElement("p")
        p.className = "RowData"
        p.innerHTML = student.FirstName
        row.appendChild(p)

        p = document.createElement("p");
        p.className = "RowData";
        p.innerHTML = student.ClassLevel;
        row.appendChild(p);

        p = document.createElement("p")
        p.className = "RowData"
        p.innerHTML = student.Address.FullAddress
        row.appendChild(p)

        // Days
        let WeekDays = ['Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday'];
                
        for (let j = 0; j < WeekDays.length; j++) {
            p = document.createElement("p");
            p.className = "RowData";

            if (student.Days[WeekDays[j]] === 1)
                p.className += " OnDay";
            else 
                p.className += " OffDay";

            row.appendChild(p);
        }

        table.appendChild(row);
    }
    MainInfo.appendChild(table);
}

function DisplayBusMap(Students) {
    let studentsToPlot = [];

    for (let i = 0; i < Students.length; i++) {
        let student = Students[i];

        studentsToPlot.push({
            Name: student.Name,
            Addresses: [student.Address],
            Order: (i + 1).toString(),
            Schedules: [student.BusSchedule]
        })
    }

    return studentsToPlot;
}

function GenerateScheduleButtons() {
    let selectordropdown = document.getElementById("ScheduleSelectorDropDown");
    selectordropdown.innerHTML = "";

    let DayPart = GetActiveDayPart().innerHTML;
    let sql;
    let ScheduleCount;

    switch(DayPart) {
        case "Morning":
            ScheduleCount = 2;
            sql = "Select Count(distinct(BusSchedule)) as Count From Student Where Length(BusSchedule) > 1 and BusSchedule Like '%Π%'";
            break;
        case "Noon":
            ScheduleCount = 3;
            sql = "Select Count(distinct(BusSchedule)) as Count From Student Where Length(BusSchedule) > 1 and BusSchedule Like '%Μ%'"
            break;
        case "Study":
            sql = "Select distinct(BusSchedule) From Student Where Length(BusSchedule) = 1 Order By BusSchedule"
            break;
        default: break;
    }

    let option = document.createElement("option");
        option.className = "ScheduleSelectorOption";
        option.value = "";
        option.innerHTML = "";
        selectordropdown.appendChild(option);

    if (DayPart === "Study") {
        
        db.each(sql, function(err, row) {
            let option = document.createElement("option");
            option.className = "ScheduleSelectorOption";
            option.value = row.BusSchedule;
            option.innerHTML = row.BusSchedule;
            selectordropdown.appendChild(option);
        })
    }
    else {
        // let Count;
        // db.each(sql, function(err, row) {
        //     Count = row.Count;
        // })

        // ScheduleCount = Math.floor(Count / 32);

        for (let i = 0; i < ScheduleCount; i++) {
            let option = document.createElement("option");
            option.className = "ScheduleSelectorOption";
            option.value = i + 1;
            option.innerHTML = i + 1;
            selectordropdown.appendChild(option);
        }
    }
    
}

function GenerateBusButtons() {
    const BusButtons = document.getElementsByClassName("BusButtonsContainer")[0];

    let sql = "Select Number From Bus Order By Number";

    db.each(sql, function(err, row) {
        const newButton = document.createElement("button");
        newButton.type = "button";
        newButton.className = "BusButton";
        newButton.id = row.Number;
        newButton.innerHTML = row.Number;

        BusButtons.appendChild(newButton);
    });
}

function GetBusFromDB(sql) {
    let Students = [];

    db.each(sql, function(err, row) {
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

            Address: {
                FullAddress: row.FullAddress,
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
            ScheduleOrder: row.ScheduleOrder
        }

        Students.push(student);
    })

    return Students
}

function CheckDisabledScheduleButton(tab) {
    if (tab == null) {
        document.getElementById("AddScheduleButton").disabled = true;
        document.getElementById("CalculateDurationButton").disabled = true;
        return;
    }

    if (tab.type === "Schedule") {
        document.getElementById("AddScheduleButton").disabled = false;
        document.getElementById("CalculateDurationButton").disabled = false;
    }
    else {
        document.getElementById("AddScheduleButton").disabled = true;
        document.getElementById("CalculateDurationButton").disabled = true;
    }
    return;
}

// Student Searching

let CurrentStudents = {};

function GetStudentFromDB(sql) {
    let Students = {};

    db.each(sql, function(err, row) {
        let id = "\"" + row.StudentID + "\"";
        // If student has not already been saved save it
        if (!Students.hasOwnProperty(id)) {
            // ClassLevel: "Β - ΔΗΜΟΤΙΚΟΥ" or "ΝΗΠΙΑΓΩΓΕΙΟ"
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

                MorningAddresses: [],
                MorningDays: [],
                MorningNotes: [],
                MorningBuses: [],
                MorningOrder: [],

                NoonAddresses: [],
                NoonDays: [],
                NoonNotes: [],
                NoonBuses: [],
                NoonOrder: [],

                StudyAddresses: [],                   
                StudyDays: [],
                StudyNotes: [],
                StudyBuses: [],
                StudyOrder: []
            };
            
            let key = row.DayPart + 'Addresses';
            student[key].push({ FullAddress: row.FullAddress,
                                AddressID: row.AddressID,
                                Longitude: row.GPS_X,
                                Latitude: row.GPS_Y } );

            key = row.DayPart + 'Days';
            student[key].push( {    Monday: row.Monday,
                                Tuesday: row.Tuesday,
                                Wednesday: row.Wednesday,
                                Thursday: row.Thursday,
                                Friday: row.Friday }  );
            
            key = row.DayPart + 'Notes';
            student[key].push(row.FullNote);

            key = row.DayPart + 'Buses';
            student[key].push(row.BusSchedule);
            
            key = row.DayPart + "Order";
            student[key].push(row.ScheduleOrder);

            Students[id] = student;
        }
        else {
            // Ιf student exists save only its DIFFERENT addresses, days, notes, buses PER daypart.
            let key = row.DayPart + 'Addresses';
            Students[id][key].push({    FullAddress: row.FullAddress,
                                        StudentID: row.AddressID,
                                        Longitude: row.GPS_X,
                                        Latitude: row.GPS_Y } );

            key = row.DayPart + 'Days';
            Students[id][key].push( {   Monday: row.Monday,
                                        Tuesday: row.Tuesday,
                                        Wednesday: row.Wednesday,
                                        Thursday: row.Thursday,
                                        Friday: row.Friday }  );
            
            key = row.DayPart + 'Notes';
            Students[id][key].push(row.Notes);

            key = row.DayPart + 'Buses';
            Students[id][key].push(row.BusSchedule)
        }
    });

    return Students;
}


function DisplayStudentSearchTable(Students) {

    // Display Students given in parameter in a table inside "MainInfo".
    var table = document.createElement("div");
    table.className = "Table";

    var firstRow = document.createElement("div");
    firstRow.className = "StudentTableRow TableRow";

    var Headers = [' ', 'Index', 'Last Name', 'First Name', 'ClassLevel'];

    // Use Headers above to create the first row of the table, showing column Names
    for (var i = 0; i < Headers.length; i++) {
        var p = document.createElement("p");
        p.className = "RowHeader";
        p.innerHTML = Headers[i];
        firstRow.appendChild(p);
    }

    table.appendChild(firstRow);

    // Basically for student in Students:
    StudentKeys = Object.keys(Students);
    for (let i = 0; i < StudentKeys.length; i++) {
        key = StudentKeys[i];
        var row = document.createElement("div");
        row.className = "StudentTableRow TableRow";
        
        // Keep the ID in html level but hidden,
        // so it can be retrieved afterwards for the "More" button
        let p = document.createElement("p");
        p.className = "RowData";
        p.innerHTML = Students[key].ID;
        p.hidden = true;
        row.appendChild(p);

        // The "More" button
        let button = document.createElement("button");
        button.type =  "button";
        button.classList.add("RowData");
        button.classList.add("MoreButton");
        button.onclick = OnMorePress;
        let searchimg = document.createElement("img");
        searchimg.src = "../images/General/more.png";
        searchimg.className = "MoreButtonImage";
        button.appendChild(searchimg);
        row.appendChild(button);

        // Index
        p = document.createElement("p");
        p.className = "RowData";
        p.innerHTML = i + 1;
        row.appendChild(p);
      
        // Last Name
        p = document.createElement("p");
        p.className = "RowData";
        p.innerHTML = Students[key].LastName;
        row.appendChild(p);

        // First Name
        p = document.createElement("p");
        p.className = "RowData";
        p.innerHTML = Students[key].FirstName;
        row.appendChild(p);

        // Class - Level
        p = document.createElement("p");
        p.className = "RowData";
        p.innerHTML = Students[key].ClassLevel;
        row.appendChild(p);

        table.appendChild(row);
    }
        
    MainInfo.appendChild(table)
}

    // This does not actually display map. It just saves info for when map will be displayed
function DisplayStudentSearchMap(Students) {
    let studentsToPlot = [];

    StudentKeys = Object.keys(Students);
    for (let i = 0; i < StudentKeys.length; i++) {
        // Find the id = key to Students "dict"
        let student = Students[StudentKeys[i]];
        let id = "\"" + student.ID + "\"";

        // *Subject to change* //
        // Add all addresses to one array, so it is easier to plot them all together.
        let Addresses = [];
        Addresses.push.apply(Addresses, student.MorningAddresses);
        Addresses.push.apply(Addresses, student.NoonAddresses);
        Addresses.push.apply(Addresses, student.StudyAddresses);

        let Schedules = [];
        Schedules.push.apply(Schedules, student.MorningBuses);
        Schedules.push.apply(Schedules, student.NoonBuses);
        Schedules.push.apply(Schedules, student.StudyBuses);

        studentsToPlot.push({
            Name: student.LastName + ' ' + student.FirstName,
            Addresses: Addresses,
            Schedules: Schedules,
            Order: ""
         });
    }

    return studentsToPlot;
}

    // More Button Click Handler
function OnMorePress() {
    let children = this.parentNode.childNodes;
    let id = children[0].innerHTML;
    id = "\"" + id + "\"";

    var student;
    var CurrentStudents = SearchTabGroup.activeTab().students;

    let exists = false;
    // If students exist in active tab's students get it from those.
    if (CurrentStudents.hasOwnProperty(id)) {
        student = CurrentStudents[id];
        exists = true;
    }
    // else get a new one
    else {
        id = children[0].innerHTML;
        let sql = "Select * From Student, Address\
        Where Student.AddressID = Address.AddressID and\
        Student.StudentID = \"" + id + "\"";

        id = "\"" + id + "\"";
        
        student = GetStudentFromDB(sql);
    }

    setTimeout(() => {     
        if (!exists) {
            student = student[id];
        }

        // Open a new search tab holding the specific student's info and Addressmap.
        let title = student.LastName + " " + student.FirstName;
        let type = "StudentCard"
        let newSearchTab = OpenSearchTab(docmain, title, type, DisplayStudentCard, DisplayStudentMap, student);
        newSearchTab.activate(false);

    }, 5);
    
    

}

    // Filters handler and Tab Creator
function SearchStudents() {
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

    const SearchValues = [FirstName, LastName, Class, Level, Street, Number, Municipal, ZipCode];
    const SearchFields = ["Student.FirstName", "Student.LastName", "Student.Class", "Student.Level", "Address.Road", "Address.Number", "Address.Municipal", "Address.ZipCode"];
    let toSearch = "Where Student.AddressID = Address.AddressID";

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
    for (var i = 0; i < SearchValues.length; i++) {
        if (SearchValues[i]) {
            SearchValues[i] = SearchValues[i].toUpperCase();
            toSearch += (" and " + SearchFields[i] + " Like \"%" + SearchValues[i] + "%\"");
        }
    }

    DayParts = ['Morning', 'Noon', 'Study'];
    
    

    let sql = "Select *\
            From Student, Address " + toSearch + " Order By Student.LastName";

    // Execute query and get Students
    let Students = GetStudentFromDB(sql);

    // Apparently Students object has no values at this point.
    // If window waits for 10 MILLIseconds everything is fine. for some reason...

    // Open a new SearchTab, which displays Students as a Table and Map Tabs.
    let title;
    let waitTime;
    if (empty) {
        title = "All Students";
        waitTime = 50;
    }
    else {
        // Student Search : 
        title = "\"" + FirstName + " " + LastName + " " + Class + " " + Level + " " + Street + " " + Number + " " + Municipal + " " + ZipCode + "\"";
        waitTime = 20;
    }
    let type = "Student"
    setTimeout(() => {     
        let newSearchTab = OpenSearchTab(docmain, title, type, DisplayStudentSearchTable, DisplayStudentSearchMap, Students);
        newSearchTab.activate(false);

        // Assign onclick to More Buttons.
        ReassignAllButtons(); 
        CurrentStudents = Students;
    }, waitTime);
}

// Clear search bars click handler
function ClearSearchBars() {
    document.getElementById("FirstNameBar").value = "";
    document.getElementById("LastNameBar").value = "";
    document.getElementById("ClassBar").value = "";
    document.getElementById("LevelBar").value = "";
}

// Onclick assignment
function OnSearchClearStudent() {   
    // Assign OnClick functions to search-clear buttons.
    var StudentSearchButton = document.getElementById("StudentSearchButton");
    StudentSearchButton.onclick = SearchStudents;

    var StudentClearButton = document.getElementById("StudentClearButton");
    StudentClearButton.onclick = ClearSearchBars;
}


// Specific Student Info

function DisplayStudentCard(student) {
    // Card is divided in 3 sections: Info, Phones and Schedules
    var StCard = document.createElement("div")
    StCard.className = "StudentCard"

    // General Info
    {
        // General Info Header
        var GIHeader = document.createElement("h2");
        GIHeader.id = "GeneralInfoHeader";
        GIHeader.className = "StudentDataHeader";
        GIHeader.innerHTML = "General Info";
        StCard.appendChild(GIHeader);

        // General Info Table of rows
        var GInfo = document.createElement("div");
        GInfo.className = "GeneralInfo";

        // Last Name
        var lab = document.createElement("label");
        lab.innerHTML = "Last Name";
        GInfo.appendChild(lab);
        p = document.createElement("p");
        p.innerHTML = student.LastName;
        GInfo.appendChild(p);

        // First Name
        lab = document.createElement("label");
        lab.innerHTML = "First Name";
        GInfo.appendChild(lab);
        var p = document.createElement("p");
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
        var PHeader = document.createElement("h2");
        PHeader.id = "PhoneHeader";
        PHeader.className = "StudentDataHeader";
        PHeader.innerHTML = "Contact Phones";
        StCard.appendChild(PHeader);

        var Phones = document.createElement("div");
        Phones.className = "StudentPhones";
        

        // Phone
        var lab = document.createElement("label");
        lab.innerHTML = "Phone";
        Phones.appendChild(lab);
        var p = document.createElement("p");
        if (student.Phone) {
            p.innerHTML = student.Phone;
        }
        else {
            p.innerHTML = "-";
        }
        Phones.appendChild(p);


        // Mobile Phone
        var lab = document.createElement("label");
        lab.innerHTML = "Mobile";
        Phones.appendChild(lab);
        var p = document.createElement("p");
        if (student.Mobile) {
            p.innerHTML = student.Mobile
        }
        else {
            p.innerHTML = "-";
        }
        Phones.appendChild(p);


        // Other Phone 2
        var lab = document.createElement("label");
        lab.innerHTML = "Other Phone 1";
        Phones.appendChild(lab);
        var p = document.createElement("p");
        if (student.OtherPhone1) {
            p.innerHTML = student.OtherPhone1;
        }
        else {
            p.innerHTML = "-";
        }
        Phones.appendChild(p);


        // Other Phone 2
        var lab = document.createElement("label");
        lab.innerHTML = "Other Phone 2";
        Phones.appendChild(lab);
        var p = document.createElement("p");
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
        var Schedules = document.createElement("div");
        Schedules.className = "StudentSchedules";

        const DayParts = ['Morning', 'Noon', 'Study'];

        // We need one table for each existing DayPart in student
        DayParts.forEach(function(dayPart) {
            var scheduleDiv = document.createElement("div");
            scheduleDiv.className = "ScheduleHeaderTable";

            // Schedule Header
            var Header = document.createElement("h2");
            Header.id = dayPart + "SchedulesHeader";
            Header.className = "StudentSchedulesHeader";
            Header.innerHTML = dayPart + " Schedules";
            scheduleDiv.appendChild(Header);

            var table = document.createElement("div");
            table.className = "Table";

            // Create the first row of headers
            var firstRow = document.createElement("div");
            firstRow.className = "SchedulesTableRow TableRow";
            
            var Headers = ['Bus Number', 'Pickup Order', 'Address', 'Note', 'Mon', 'Tue', 'Wen', 'Thu', 'Fri'];

            for (var i = 0; i < Headers.length; i++) {
                var p = document.createElement("p");
                p.className = "RowHeader";
                p.innerHTML = Headers[i];
                firstRow.appendChild(p);
            }

            table.appendChild(firstRow);

            
            let Addresses = dayPart + 'Addresses';
            let Notes = dayPart + 'Notes';
            let Days = dayPart + 'Days';
            let Buses = dayPart + 'Buses';
            let Orders = dayPart + 'Order';
            
            // If dayPart does not exist in students create a "p" with nothing in it
            if (student[Addresses].length === 0) {
                var p = document.createElement("p");
                p.className = "RowData";
                p.innerHTML = "-";
                scheduleDiv.appendChild(p);
            }


            for (i = 0; i < student[Addresses].length; i++) {
                var row = document.createElement("div");
                row.className = "SchedulesTableRow TableRow";

                // Bus Number
                p = document.createElement("p");
                p.className = "RowData";
                if (student[Buses][i])
                    p.innerHTML = student[Buses][i];
                else 
                    p.innerHTML = "-"
                row.appendChild(p)

                // Schedule Order (at what index student will be picked up)
                p = document.createElement("p");
                p.className = "RowData";
                if (student[Orders][i])
                    p.innerHTML = student[Orders][i];
                else 
                    p.innerHTML = "-"
                row.appendChild(p);

                // Address
                p = document.createElement("p");
                p.className = "RowData";
                p.innerHTML = student[Addresses][i].FullAddress;
                row.appendChild(p);

                // Notes
                p = document.createElement("p");
                p.className = "RowData";
                if (student[Notes][i])
                    p.innerHTML = student[Notes][i];
                else
                    p.innerHTML = "-";
                row.appendChild(p);
                
                // Days
                let WeekDays = ['Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday'];
                
                for (let j = 0; j < WeekDays.length; j++) {
                    p = document.createElement("p");
                    p.className = "RowData";

                    if (student[Days][i][WeekDays[j]] === 1)
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
    let Addresses = [];
    Addresses.push.apply(Addresses, student.MorningAddresses);
    Addresses.push.apply(Addresses, student.NoonAddresses);
    Addresses.push.apply(Addresses, student.StudyAddresses);

    let Schedules = [];
    Schedules.push.apply(Schedules, student.MorningBuses);
    Schedules.push.apply(Schedules, student.NoonBuses);
    Schedules.push.apply(Schedules, student.StudyBuses);

    studentsToPlot.push( {
        Name: student.LastName + ' ' + student.FirstName,
        Addresses: Addresses,
        Schedules: Schedules,
        Order: ""
     });

    return studentsToPlot;
}



// Search Tabs
// Each search tab will have 2 sub tabs

function OpenSearchTab(element, title, type, infoDisplayFunction, mapDisplayFunction, students) {
    // Create a new Search Tab and Display it.
    let newTab = new Tab([element], title, type, true);
    SearchTabGroup.addTab(newTab, OnSearchTabPress, OnCloseTabPress);
    newTab.students = students;

    // Depending on the process we want to do, we give different functions
    // for info and map display.
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

    // Create a subTabGroup in tab to hold Info and Map
    let InfoMapTabGroup = new TabGroup(InfoMapTabHeader);
    tab.subTabGroup = InfoMapTabGroup;

    // Display the info and store it to a new InfoTab
    tab.infoDisplayFunction(tab.students);
    let InfoTab = new Tab([MainInfo], "Info", "Info", false);
    InfoMapTabGroup.addTab(InfoTab, OnInfoTabPress);

    // Reset html to create a new MapTab
    MainInfo.innerHTML = "";
    // Use mapDisplayFunction to get all the addresses we want to plot
    tab.studentsToPlot = tab.mapDisplayFunction(tab.students);

    // Create a new MapTab to hold the map
    // Note: This does not actually hold the map. Rather, it will render the map when it is pressed.

    let MapTab = new Tab([MainInfo], "Map", "Map", false);
    InfoMapTabGroup.addTab(MapTab, OnMapTabPress);

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
}

    // Click Handlers
function OnSearchTabPress() {
    // When search Tab is pressed display it and activate it.
    // Note: Displaying here is important because buttons(onclicks) and maps cannot be stored in any other way.
    let pressedTab = SearchTabGroup.getPressed(this);
    if (pressedTab === SearchTabGroup.activeTab() || pressedTab.closed || closing) {
        closing = false;
        return;
    }

    DisplaySearchTab(pressedTab);
    pressedTab.activate(false);

    CheckDisabledScheduleButton(pressedTab)

    ReassignAllButtons();
}

function OnClearTabsPress() {
    SearchTabGroup.clearTabs();

    CacheDOM();
    InfoMapTabHeader.innerHTML = "";
    MainInfo.innerHTML = "";

    CheckDisabledScheduleButton(null);
}

function OnCloseTabPress() {
    closing = true;
    let a = SearchTabGroup.closePressed(this);
    if (SearchTabGroup.length === 0) {
        InfoMapTabHeader.innerHTML = "";
        MainInfo.innerHTML = "";
    }
    else {
        let active = SearchTabGroup.activeTab();
        DisplaySearchTab(SearchTabGroup.activeTab());
        SearchTabGroup.activeTab().activate(false);
        CheckDisabledScheduleButton(SearchTabGroup.activeTab())
        ReassignAllButtons();
    }
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

// Map functions

function CreateMap(tab) {
    let Students = tab.studentsToPlot;
    // Create an empty map:
    let coords = [];
    // StudentKeys = Object.keys(Students);
    for (let i = 0; i < Students.length; i++) {
        let student = Students[i];
        for (let j = 0; j < student.Addresses.length; j++) {
            let address = student.Addresses[j];
            coords.push([address.Latitude, address.Longitude]);
        }
    }

    // Find a "centroid" from all addresses given so map can center there.
    let Ox = 0;
    let Oy = 0;
    coords.forEach(element => {
        Ox += element[0];
        Oy += element[1];
    });

    Ox /= coords.length;
    Oy /= coords.length;
    
    // map is a global variable which hold the CURRENT open map.
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
    
    // For each student add a marker to the current open map.
    // StudentKeys = Object.keys(Students);
    let plottedAddresses = {};
    for (let i = 0; i < Students.length; i++) {
        let student = Students[i];
        let studentName = student.Name;

        for (let j = 0; j < student.Addresses.length; j++) {
            let address = student.Addresses[j];
            let schedule = student.Schedules[j];
            let key = address.Latitude + "," + address.Longitude;
            let found = false;

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

                title += (index + ") " + studentName + "\n");
                title += address.FullAddress + "\n";
                title += schedule;
                plottedAddresses[key].names.push(studentName)
                plottedAddresses[key].title = title;              

            }
            else {
                plottedAddresses[key] = {
                    title: "1) " + studentName + "\n" + address.FullAddress + "\n" + schedule,
                    names: [studentName],
                    address: address,
                    schedule: schedule,
                    order: student.Order
                };
            }
        }
    }

    let plottedAddressKeys = Object.keys(plottedAddresses);
    for (let i = 0; i < plottedAddressKeys.length; i++) {
        let toMark = plottedAddresses[plottedAddressKeys[i]];

        let icon
        if (toMark.order === "")
            icon = defaultIcon;
        else {
            icon = {
                url: "../images/NumberedMarkers/marker" + toMark.order + ".png",
                size: {width: 26, height: 32},
                origin: {x: 0, y: 0},
                anchor: {
                    x: "-16px",
                    y: "-32px"
                }
            }
        }

        var marker = new khtml.maplib.overlay.Marker({
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
    for (let i = 0; i < Schedules.length; i++) {
        let icon = {
            url: Markers[i],
            size: {width: 26, height: 32},
            origin: {x: 0, y: 0},
            anchor: {
                x: "-16px",
                y: "-32px"
            }
        };

        SchedulesIcons[Schedules[i]] = {
            Schedule: Schedules[i],
            Icon: icon
        }
    }
    // For each student add a marker to the current open map, depending on student's schedule
    // StudentKeys = Object.keys(Students);
    let plottedAddresses = {};
    for (let i = 0; i < Students.length; i++) {
        let student = Students[i];
        let studentName = student.Name;

        for (let j = 0; j < student.Addresses.length; j++) {
            let address = student.Addresses[j];
            let schedule = student.Schedules[j];
            let key = address.Latitude + "," + address.Longitude;
            let found = false;

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

                title += (index + ") " + studentName + "\n");
                title += address.FullAddress + "\n";
                title += schedule;
                plottedAddresses[key].names.push(studentName)
                plottedAddresses[key].title = title;              

            }
            else {
                plottedAddresses[key] = {
                    title: "1) " + studentName + "\n" + address.FullAddress + "\n" + schedule,
                    names: [studentName],
                    address: address,
                    schedule: schedule,
                    order: student.Order
                };
            }
        }
    }

    let plottedAddressKeys = Object.keys(plottedAddresses);
    for (let i = 0; i < plottedAddressKeys.length; i++) {
        let toMark = plottedAddresses[plottedAddressKeys[i]];

        var marker = new khtml.maplib.overlay.Marker({
            position: new khtml.maplib.LatLng(toMark.address.Latitude, toMark.address.Longitude), 
            map: map,
            title: toMark.title,
            names: toMark.names,
            address: toMark.address,
            schedule: toMark.schedule,
            icon: SchedulesIcons[toMark.schedule].Icon
        });
    }
}

// General Utility functions

function ReassignAllButtons() {
    var moreButtons = document.getElementsByClassName("MoreButton");

    for (let i = 0; i < moreButtons.length; i++) {
        moreButtons[i].onclick = OnMorePress;
    }

    // Assign OnClick functions to search-clear buttons.
    var StudentSearchButton = document.getElementById("StudentSearchButton");
    StudentSearchButton.onclick = SearchStudents;

    var StudentClearButton = document.getElementById("StudentClearButton");
    StudentClearButton.onclick = ClearSearchBars;

    OnBusClickHandle();
}

function CacheDOM() {
    // Caching DOM elements.
    docmain = document.getElementsByTagName("main")[0];
    SearchTabHeader = document.getElementsByClassName("SearchTabGroup")[0];
    InfoMapTabHeader = document.getElementsByClassName("InfoMapTabGroup")[0];
    MainInfo = document.getElementsByClassName("MainInfo")[0];
}

// Loader
function OnCreateWindow() {
    // pythondir = "../python/"
    // datadir = "../data/"

    sqlite3 = require('sqlite3').verbose();
    console.log(datadir + "MMGP_data.db");
    db = new sqlite3.Database(datadir + "MMGP_data.db");
    
    GenerateBusButtons();
    OnSearchClearStudent();

    CacheDOM();

    // Create a SearchTabGroup and hold it to a global variable for use.
    SearchTabGroup = new TabGroup(document.getElementsByClassName("SearchTabGroup")[0])

    document.getElementById("ClearTabsButton").onclick = OnClearTabsPress;

    ReassignAllButtons();

    console.log(__dirname)
}