// External modules
let sqlite3;
let db;
let DOMElementHistory;

// TabGroups
let SearchTabGroup;

// Dom Elements
let InfoMapTabHeader;
let SearchTabHeader;
let MainInfo;
let docmain;

let currentOpenBus = '0';
//let studentsToPlot;

let docheader;
let mainHistory;
let headerHistory;

let map;

function PullBuses() {
    if (this.innerHTML === currentOpenBus) {
        return;
    }

    currentOpenBus = this.innerHTML;


    var table = document.createElement("div")
    table.className = "Table"


    // Create the first row of headers
    var firstRow = document.createElement("div")
    firstRow.className = "busTableRow"

    var Headers = [' ', 'Bus Number', 'First Name', 'Last Name', 'Address', 'Mon', 'Tue', 'Wen', 'Thu', 'Fri']

    for (var i = 0; i < Headers.length; i++) {
        var p = document.createElement("p")
        p.className = "rowHeader"
        p.innerHTML = Headers[i]
        firstRow.appendChild(p)
    }

    table.appendChild(firstRow);

    // Pull from sql and print out
    // For now test:
    var Firsts = ['Giannis', 'Dimitris', 'Vasilis', 'Giorgos', 'Nikos', 'Kostas']
    var Lasts = ['Ioannou', 'Dimitriou', 'Vasileiou', 'Georgiou', 'Nikolaou', 'Konstantinou']
    var Add = 'Erechthiou 5, Alimos 17455'

    for (var studs = 0; studs < 50; studs++) {
        var row = document.createElement("div")
        row.className = "busTableRow"

        let first = Math.floor(Math.random() * 6)
        let last = Math.floor(Math.random() * 6)

        let button = document.createElement("button")
        button.type =  "button";
        button.className = "rowData"
        let searchimg = document.createElement("img");
        searchimg.src = "../Images/more.png";
        searchimg.className = "searchButtonImage"
        button.appendChild(searchimg);
        row.appendChild(button);

        let p = document.createElement("p")
        p.className = "rowData"
        p.innerHTML = this.innerHTML
        row.appendChild(p)
        
        p = document.createElement("p")
        p.className = "rowData"
        p.innerHTML = Firsts[first]
        row.appendChild(p)

        p = document.createElement("p")
        p.className = "rowData"
        p.innerHTML = Lasts[last]
        row.appendChild(p)

        p = document.createElement("p")
        p.className = "rowData"
        p.innerHTML = Add
        row.appendChild(p)

        for (var i = 0; i < 5; i++) {
            rand = Math.random() * 100;
            var day = true;
            if (rand > 50) day = true;
            else day = false;

            p = document.createElement("p")
            p.className = "rowData"
            if (day === true)
                p.className += " onDay"
            else 
                p.className += " offDay"
            row.appendChild(p)
        }


        table.appendChild(row)
    }

    MainInfo.appendChild(table)

    // let InfoTab = new DOMElementHistory.Tab(docmain, "Info", false);
    let InfoTab = new Tab(docmain, "Info", false);
    InfoMapTabGroup.addTab(InfoTab, OnTabPress);

    // docmain.innerHTML = "";
    // let MapTab = new DOMElementHistory.Tab(docmain, "Map", false);
    let MapTab = new Tab(docmain, "Map", false);
    InfoMapTabGroup.addTab(MapTab, OnTabPress);

    InfoTab.activate();

    // mainHistory.saveState();
    // headerHistory.saveState();
}

function DisplayBusMap() {

}

function OnClickBus() {
    var BusButtons = document.getElementsByClassName("bus");

    for (let i = 0; i < BusButtons.length; i++) {
        BusButtons[i].onmouseup = PullBuses;
    }
}

function GenerateBusButtons() {
    const BusButtons = document.getElementsByClassName("BusButtons")[0];

    for (var i = 1; i <= 32; i++) {

        const newButton = document.createElement("button");
        newButton.type = "button";
        newButton.className = "bus";
        newButton.innerHTML = i.toString();

        BusButtons.appendChild(newButton);
    }
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
                                Longitude: row.GPS_X,
                                Latitude: row.GPS_Y } );

            key = row.DayPart + 'Days';
            student[key].push( {    Monday: row.Monday,
                                    Tuesday: row.Tuesday,
                                    Wednesday: row.Wednesday,
                                    Thursday: row.Thursday,
                                    Friday: row.Friday }  );
            
            key = row.DayPart + 'Notes';
            student[key].push(row.Notes);

            key = row.DayPart + 'Buses';
            student[key].push(row.BusSchedule);

            Students[id] = student;
        }
        else {
            // Ιf student exists save only its DIFFERENT addresses, days, notes, buses PER daypart.
            let key = row.DayPart + 'Addresses';
            Students[id][key].push({    FullAddress: row.FullAddress,
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
    })

    return Students;
}


function DisplayStudentSearchTable(Students) {

    // Display Students given in parameter in a table inside "MainInfo".
    var table = document.createElement("div");
    table.className = "Table";

    var firstRow = document.createElement("div");
    firstRow.className = "studentTableRow";

    var Headers = [' ', 'Index', 'Last Name', 'First Name', 'ClassLevel'];

    // Use Headers above to create the first row of the table, showing column Names
    for (var i = 0; i < Headers.length; i++) {
        var p = document.createElement("p");
        p.className = "rowHeader";
        p.innerHTML = Headers[i];
        firstRow.appendChild(p);
    }

    table.appendChild(firstRow);

    // Basically for student in Students:
    StudentKeys = Object.keys(Students);
    for (let i = 0; i < StudentKeys.length; i++) {
        key = StudentKeys[i];
        var row = document.createElement("div");
        row.className = "studentTableRow";
        
        // Keep the ID in html level but hidden,
        // so it can be retrieved afterwards for the "More" button
        let p = document.createElement("p");
        p.className = "rowData";
        p.innerHTML = Students[key].ID;
        p.hidden = true;
        row.appendChild(p);

        // The "More" button
        let button = document.createElement("button");
        button.type =  "button";
        button.classList.add("rowData");
        button.classList.add("MoreButton");
        button.onclick = OnMorePress;
        let searchimg = document.createElement("img");
        searchimg.src = "../Images/more.png";
        searchimg.className = "searchButtonImage";
        button.appendChild(searchimg);
        row.appendChild(button);

        // Index
        p = document.createElement("p");
        p.className = "rowData";
        p.innerHTML = i + 1;
        row.appendChild(p);
      
        // Last Name
        p = document.createElement("p");
        p.className = "rowData";
        p.innerHTML = Students[key].LastName;
        row.appendChild(p);

        // First Name
        p = document.createElement("p");
        p.className = "rowData";
        p.innerHTML = Students[key].FirstName;
        row.appendChild(p);

        // Class - Level
        p = document.createElement("p");
        p.className = "rowData";
        p.innerHTML = Students[key].ClassLevel;
        row.appendChild(p);

        table.appendChild(row);
    }
        
    MainInfo.appendChild(table)
}

    // This does not actually display map. It just saves info for when map will be displayed
function DisplayStudentSearchMap(Students) {
    let studentsToPlot = {};

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
        studentsToPlot[id] = {
            Name: student.LastName + ' ' + student.FirstName,
            Addresses: Addresses };
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

    // If students exist in active tab's students get it from those.
    if (CurrentStudents.hasOwnProperty(id)) {
        student = CurrentStudents[id];
    }
    // else get a new one
    else {
        let sql = "Select * From Student, Address\
        Where Student.AddressID = Address.AddressID and\
        Student.StudentID = \"" + id + "\"";

        student = GetStudentFromDB(sql)[id];
    }

    // Open a new search tab holding the specific student's info and Addressmap.
    let title = "Student Info: " + student.LastName + " " + student.FirstName;
    let newSearchTab = OpenSearchTab(docmain, title, DisplayStudentCard, DisplayStudentMap, student);
    newSearchTab.activate(false);

}

    // Filters handler and Tab Creator
function SearchStudents() {
    // *SQL Injection* //

    // Grab all filters from search bars
    const FirstName = document.getElementById("FirstNameBar").value;
    const LastName = document.getElementById("LastNameBar").value;
    const Class = document.getElementById("ClassBar").value;
    const Level = document.getElementById("LevelBar").value;

    const SearchValues = [FirstName, LastName, Class, Level];
    const SearchFields = ["FirstName", "LastName", "Class", "Level"];
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
    setTimeout(() => {     
        let title = "Student Search : \"" + FirstName + " " + LastName + " " + Class + " " + Level + "\"";
        let newSearchTab = OpenSearchTab(docmain, title, DisplayStudentSearchTable, DisplayStudentSearchMap, Students);
        newSearchTab.activate(false);

        // Assign onclick to More Buttons.
        ReassignAllButtons(); 
        CurrentStudents = Students;
    }, 10);

    
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
    var StudentSearchButton = document.getElementById("studentSearchButton");
    StudentSearchButton.onclick = SearchStudents;

    var StudentClearButton = document.getElementById("studentClearButton");
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
            firstRow.className = "schedulesTableRow";
            
            var Headers = ['Bus Number', 'Pickup Order', 'Address', 'Note', 'Mon', 'Tue', 'Wen', 'Thu', 'Fri'];

            for (var i = 0; i < Headers.length; i++) {
                var p = document.createElement("p");
                p.className = "rowHeader";
                p.innerHTML = Headers[i];
                firstRow.appendChild(p);
            }

            table.appendChild(firstRow);

            
            let Addresses = dayPart + 'Addresses';
            let Notes = dayPart + 'Notes';
            let Days = dayPart + 'Days';
            
            // If dayPart does not exist in students create a "p" with nothing in it
            if (student[Addresses].length === 0) {
                var p = document.createElement("p");
                p.className = "rowData";
                p.innerHTML = "-";
                scheduleDiv.appendChild(p);
            }


            for (i = 0; i < student[Addresses].length; i++) {
                var row = document.createElement("div");
                row.className = "schedulesTableRow";

                // Bus Number
                p = document.createElement("p");
                p.className = "rowData";
                p.innerHTML = "Unassigned Bus";
                row.appendChild(p)

                // Schedule Order (at what index student will be picked up)
                p = document.createElement("p");
                p.className = "rowData";
                p.innerHTML = "Unassigned Order";
                row.appendChild(p);

                // Address
                p = document.createElement("p");
                p.className = "rowData";
                p.innerHTML = student[Addresses][i].FullAddress;
                row.appendChild(p);

                // Notes
                p = document.createElement("p");
                p.className = "rowData";
                if (student.MorningNotes[i])
                    p.innerHTML = student[Notes][i];
                else
                    p.innerHTML = "-";
                row.appendChild(p);
                
                // Days
                let WeekDays = ['Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday'];
                
                for (let j = 0; j < WeekDays.length; j++) {
                    p = document.createElement("p");
                    p.className = "rowData";

                    if (student[Days][i][WeekDays[j]] === 1)
                        p.className += " onDay";
                    else 
                        p.className += " offDay";

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
    let studentsToPlot = {};

    // Find the id = key to Students "dict"
    let id = "\"" + student.ID + "\"";

    // *Subject to change* //
    // Add all addresses to one array, so it is easier to plot them all together.
    let Addresses = [];
    Addresses.push.apply(Addresses, student.MorningAddresses);
    Addresses.push.apply(Addresses, student.NoonAddresses);
    Addresses.push.apply(Addresses, student.StudyAddresses);
    studentsToPlot[id] = {
        Name: student.LastName + ' ' + student.FirstName,
        Addresses: Addresses };

    return studentsToPlot;
}



// Search Tabs
// Each search tab will have 2 sub tabs

function OpenSearchTab(element, title, infoDisplayFunction, mapDisplayFunction, students) {
    // Create a new Search Tab and Display it.
    let newTab = new Tab([element], title, true);
    SearchTabGroup.addTab(newTab, OnSearchTabPress, OnCloseTabPress);
    newTab.students = students;

    // Depending on the process we want to do, we give different functions
    // for info and map display.
    newTab.infoDisplayFunction = infoDisplayFunction;
    newTab.mapDisplayFunction = mapDisplayFunction;

    DisplaySearchTab(newTab);
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
    let InfoTab = new Tab([MainInfo], "Info", false);
    InfoMapTabGroup.addTab(InfoTab, OnInfoTabPress);

    // Reset html to create a new MapTab
    MainInfo.innerHTML = "";
    // Use mapDisplayFunction to get all the addresses we want to plot
    tab.studentsToPlot = tab.mapDisplayFunction(tab.students);

    // Create a new MapTab to hold the map
    // Note: This does not actually hold the map. Rather, it will render the map when it is pressed.

    let MapTab = new Tab([MainInfo], "Map", false);
    InfoMapTabGroup.addTab(MapTab, OnMapTabPress);

    if (prevActive === 0) {
        InfoTab.activate();
    }
    else if (prevActive === 1) {
        MapTab.activate();
        MainInfo.innerHTML = "";

        CreateMap(tab.studentsToPlot);
        PlotStudents(tab.studentsToPlot);
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
    DisplaySearchTab(pressedTab);
    pressedTab.activate(false);
    ReassignAllButtons();
}

function OnClearTabsPress() {
    SearchTabGroup.clearTabs();

    CacheDOM();
    InfoMapTabHeader.innerHTML = "";
    MainInfo.innerHTML = "";
}

function OnCloseTabPress() {
    SearchTabGroup.closePressed(this);
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
    
    CreateMap(searchTab.studentsToPlot);
    PlotStudents(searchTab.studentsToPlot);
}

// Map functions

function CreateMap(Students) {
    // Create an empty map:
    let coords = [];
    StudentKeys = Object.keys(Students);
    for (let i = 0; i < StudentKeys.length; i++) {
        let student = Students[StudentKeys[i]];
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

function PlotStudents(Students) {
    // Define icon to render
    let icon = {
		url: "../images/red-dot.png",
		size: {width: 26, height: 32},
		origin: {x: 0, y: 0},
		anchor: {
			x: "-10px",
			y: "-32px"
		}
	};
    
    // For each student add a marker to the current open map.
    StudentKeys = Object.keys(Students);
    let plottedAddresses = {};
    for (let i = 0; i < StudentKeys.length; i++) {
        let student = Students[StudentKeys[i]];
        let studentName = student.Name;

        for (let j = 0; j < student.Addresses.length; j++) {
            let address = student.Addresses[j];
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
                title += address.FullAddress;
                plottedAddresses[key].names.push(studentName)
                plottedAddresses[key].title = title;              

            }
            else {
                plottedAddresses[key] = {
                    title: "1) " + studentName + "\n" + address.FullAddress,
                    names: [studentName],
                    address: address
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
            icon: icon
        });
    }
    
    console.log(plottedAddresses);
}

// General Utility functions

function ReassignAllButtons() {
    var moreButtons = document.getElementsByClassName("MoreButton");

    for (let i = 0; i < moreButtons.length; i++) {
        moreButtons[i].onclick = OnMorePress;
    }

    // Assign OnClick functions to search-clear buttons.
    var StudentSearchButton = document.getElementById("studentSearchButton");
    StudentSearchButton.onclick = SearchStudents;

    var StudentClearButton = document.getElementById("studentClearButton");
    StudentClearButton.onclick = ClearSearchBars;
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

    sqlite3 = require('sqlite3').verbose();
    db = new sqlite3.Database('../data/MMGP_data.db');

    GenerateBusButtons();
    OnClickBus();
    OnSearchClearStudent();

    CacheDOM();

    // Create a SearchTabGroup and hold it to a global variable for use.
    SearchTabGroup = new TabGroup(document.getElementsByClassName("SearchTabGroup")[0])

    document.getElementById("ClearTabsButton").onclick = OnClearTabsPress;
}

// module.exports = {BackClick, ForwardClick};