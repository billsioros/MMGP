const sqlite3 = require('sqlite3').verbose();
const db = new sqlite3.Database('../data/MMGP_data.db');

const DOMElementHistory = require("domelementhistory")


let currentOpenBus = '0';
let docmain;
let mainHistory;

function PullBuses() {
    if (this.innerHTML === currentOpenBus) {
        return;
    }

    currentOpenBus = this.innerHTML;
    
    // Remove previously formatted table
    docmain.innerHTML = ""

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

    docmain.appendChild(table)

    mainHistory.saveState();
}

function OnClickBus() {
    var BusButons = document.getElementsByClassName("bus")

    for (let i = 0; i < BusButons.length; i++) {
        BusButons[i].onmouseup = PullBuses
    }
}

function GenerateBusButtons() {
    const BusButtons = document.getElementsByClassName("BusButtons")[0]

    for (var i = 1; i <= 32; i++) {

        const newButton = document.createElement("button")
        newButton.type = "button"
        newButton.className = "bus"
        newButton.innerHTML = i.toString();

        BusButtons.appendChild(newButton)
    }
}

// Student Searching

let CurrentStudents = {};

function GetStudentFromDB(sql) {
    let Students = {};

    db.each(sql, function(err, row) {
        let id = "\"" + row.StudentID + "\"";
        if (!Students.hasOwnProperty(id)) {

            let classLevel;
            if (!row.Class) 
                classLevel = row.Level
            else
                classLevel = row.Class + " - " + row.Level + "Y"

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
            }
            
            let key = row.DayPart + 'Addresses';
            student[key].push(row.FullAddress);

            key = row.DayPart + 'Days';
            student[key].push( {    Monday: row.Monday,
                                    Tuesday: row.Tuesday,
                                    Wednesday: row.Wednesday,
                                    Thursday: row.Thursday,
                                    Friday: row.Friday }  );
            
            key = row.DayPart + 'Notes';
            student[key].push(row.Notes);

            key = row.DayPart + 'Buses';
            student[key].push(row.BusSchedule)

            Students[id] = student;
        }
        else {
            let key = row.DayPart + 'Addresses';
            Students[id][key].push(row.FullAddress);

            key = row.DayPart + 'Days';
            Students[id][key].push( {    Monday: row.Monday,
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

function DisplayStudentTable(Students) {
    
    // Remove previously formatted table
   docmain.innerHTML = ""

    var table = document.createElement("div")
    table.className = "Table"

    var firstRow = document.createElement("div")
    firstRow.className = "studentTableRow"

    var Headers = [' ', 'Index', 'Last Name', 'First Name', 'ClassLevel']

    for (var i = 0; i < Headers.length; i++) {
        var p = document.createElement("p")
        p.className = "rowHeader"
        p.innerHTML = Headers[i]
        firstRow.appendChild(p)
    }

    table.appendChild(firstRow);

    StudentKeys = Object.keys(Students);
    for (let i = 0; i < StudentKeys.length; i++) {
        key = StudentKeys[i];
        var row = document.createElement("div")
        row.className = "studentTableRow"

        let p = document.createElement("p")
        p.className = "rowData"
        p.innerHTML = Students[key].ID;
        p.hidden = true;
        row.appendChild(p)

        let button = document.createElement("button")
        button.type =  "button";
        button.classList.add("rowData")
        button.classList.add("MoreButton")
        button.onclick = OnMorePress;
        let searchimg = document.createElement("img");
        searchimg.src = "../Images/more.png";
        searchimg.className = "searchButtonImage"
        button.appendChild(searchimg);
        row.appendChild(button);

        p = document.createElement("p")
        p.className = "rowData";
        p.innerHTML = i + 1;
        row.appendChild(p)
      
        p = document.createElement("p")
        p.className = "rowData"
        p.innerHTML = Students[key].LastName
        row.appendChild(p)

        p = document.createElement("p")
        p.className = "rowData"
        p.innerHTML = Students[key].FirstName
        row.appendChild(p)

        p = document.createElement("p")
        p.className = "rowData"
        p.innerHTML = Students[key].ClassLevel
        row.appendChild(p)

        table.appendChild(row)
    }
        
    docmain.appendChild(table)
}

function SearchStudents() {
    var header = document.getElementsByTagName("header")[0]
    var FirstName = document.getElementById("FirstNameBar").value;
    var LastName = document.getElementById("LastNameBar").value;
    var Class = document.getElementById("ClassBar").value;
    var Level = document.getElementById("LevelBar").value;

    const SearchValues = [FirstName, LastName, Class, Level]
    const SearchFields = ["FirstName", "LastName", "Class", "Level"]
    let toSearch = "Where Student.AddressID = Address.AddressID"

    let empty = true;
    SearchValues.forEach(function(value) {
        if (value) {
            empty = false;
        }
    })

    if (empty) {
        if(!confirm("You have given no filters. This will display the whole database. Are you sure?"))
            return;
    }

    for (var i = 0; i < SearchValues.length; i++) {
        if (SearchValues[i]) {
            SearchValues[i] = SearchValues[i].toUpperCase();
            toSearch += (" and " + SearchFields[i] + " Like \"%" + SearchValues[i] + "%\"");
        }
    }

    DayParts = ['Morning', 'Noon', 'Study']
    
    

    let sql = "Select *\
            From Student, Address " + toSearch;

    let Students = GetStudentFromDB(sql)

    // Apparently Students object has no values at this point
    // If window waits for 5 MILLIseconds everything is fine. for some reason...
    setTimeout(() => {DisplayStudentTable(Students);}, 5)
    CurrentStudents = Students;
    
    setTimeout(() => {mainHistory.saveState();}, 5);
}

function ClearSearchBars() {
    document.getElementById("FirstNameBar").value = "";
    document.getElementById("LastNameBar").value = "";
    document.getElementById("ClassBar").value = "";
    document.getElementById("LevelBar").value = "";
}

function OnSearchClearStudent() {   
    var StudentSearchButton = document.getElementById("studentSearchButton");
    StudentSearchButton.onmouseup = SearchStudents

    var StudentClearButton = document.getElementById("studentClearButton")
    StudentClearButton.onmouseup = ClearSearchBars
}


// Specific Student Info

function DisplayStudentCard(id) {
    // Acquire student info
    var student;
    if (CurrentStudents.hasOwnProperty(id)) {
        student = CurrentStudents[id];
    }
    else {
        let sql = "Select * From Student, Address\
        Where Student.AddressID = Address.AddressID and\
        Student.StudentID = \"" + id + "\"";

        student = GetStudentFromDB(sql)[id];
    }

    // Display student Card

    // Remove previously formatted main
    docmain.innerHTML = ""

    var StCard = document.createElement("div")
    StCard.className = "StudentCard"

    

    // General Info
    {
        var GIHeader = document.createElement("header")
        GIHeader.id = "GeneralInfoHeader"
        GIHeader.className = "StudentDataHeader"
        GIHeader.innerHTML = "General Info"
        StCard.appendChild(GIHeader);

        var GInfo = document.createElement("div")
        GInfo.className = "GeneralInfo"

        var lab = document.createElement("label")
        lab.innerHTML = "Last Name"
        GInfo.appendChild(lab)
        p = document.createElement("p")
        p.innerHTML = student.LastName
        GInfo.appendChild(p)


        lab = document.createElement("label")
        lab.innerHTML = "First Name"
        GInfo.appendChild(lab)
        var p = document.createElement("p")
        p.innerHTML = student.FirstName
        GInfo.appendChild(p)
      

        lab = document.createElement("label")
        lab.innerHTML = "Class - Level"
        GInfo.appendChild(lab)
        p = document.createElement("p")
        p.innerHTML = student.ClassLevel
        GInfo.appendChild(p)


        StCard.appendChild(GInfo)
    }

    // Phones
    {
        var PHeader = document.createElement("header")
        PHeader.id = "PhoneHeader"
        PHeader.className = "StudentDataHeader"
        PHeader.innerHTML = "Contact Phones"
        StCard.appendChild(PHeader);

        var Phones = document.createElement("div")
        Phones.className = "StudentPhones"
        
        var lab = document.createElement("label")
        lab.innerHTML = "Phone"
        Phones.appendChild(lab)
        var p = document.createElement("p")
        if (student.Phone) {
            p.innerHTML = student.Phone
        }
        else {
            p.innerHTML = "-"
        }
        Phones.appendChild(p)

        var lab = document.createElement("label")
        lab.innerHTML = "Mobile"
        Phones.appendChild(lab)
        var p = document.createElement("p")
        if (student.Mobile) {
            p.innerHTML = student.Mobile
        }
        else {
            p.innerHTML = "-"
        }
        Phones.appendChild(p)

        var lab = document.createElement("label")
        lab.innerHTML = "Other Phone 1"
        Phones.appendChild(lab)
        var p = document.createElement("p")
        if (student.OtherPhone1) {
            p.innerHTML = student.OtherPhone1
        }
        else {
            p.innerHTML = "-"
        }
        Phones.appendChild(p)

        var lab = document.createElement("label")
        lab.innerHTML = "Other Phone 2"
        Phones.appendChild(lab)
        var p = document.createElement("p")
        if (student.OtherPhone2) {
            p.innerHTML = student.OtherPhone2
        }
        else {
            p.innerHTML = "-"
        }
        Phones.appendChild(p)

        
        StCard.appendChild(Phones)
    }

    // Schedules
    {
        var Schedules = document.createElement("div")
        Schedules.className = "StudentSchedules"

        StCard.appendChild(Schedules)

        const DayParts = ['Morning', 'Noon', 'Study'];

        DayParts.forEach(function(dayPart) {
            var Header = document.createElement("header")
            Header.id = dayPart + "SchedulesHeader"
            Header.className = "StudentSchedulesHeader"
            Header.innerHTML = dayPart + " Schedules"
            Schedules.appendChild(Header)

            var table = document.createElement("div")
            table.className = "Table"

            // Create the first row of headers
            var firstRow = document.createElement("div")
            firstRow.className = "schedulesTableRow"
            
            var Headers = ['Bus Number', 'Pickup Order', 'Address', 'Note', 'Mon', 'Tue', 'Wen', 'Thu', 'Fri']

            for (var i = 0; i < Headers.length; i++) {
                var p = document.createElement("p")
                p.className = "rowHeader"
                p.innerHTML = Headers[i]
                firstRow.appendChild(p)
            }

            table.appendChild(firstRow);

            let Addresses = dayPart + 'Addresses'
            let Notes = dayPart + 'Notes'
            let Days = dayPart + 'Days'

            for (i = 0; i < student[Addresses].length; i++) {
                var row = document.createElement("div")
                row.className = "schedulesTableRow"

                p = document.createElement("p")
                p.className = "rowData"
                p.innerHTML = "Unassigned Bus"
                row.appendChild(p)

                p = document.createElement("p")
                p.className = "rowData"
                p.innerHTML = "Unassigned Order";
                row.appendChild(p)

                p = document.createElement("p")
                p.className = "rowData"
                p.innerHTML = student[Addresses][i]
                row.appendChild(p)

                p = document.createElement("p")
                p.className = "rowData"
                if (student.MorningNotes[i])
                    p.innerHTML = student[Notes][i];
                else
                    p.innerHTML = "-";
                row.appendChild(p)
                
                let WeekDays = ['Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday']
                
                for (let j = 0; j < WeekDays.length; j++) {
                    p = document.createElement("p")
                    p.className = "rowData"

                    if (student[Days][i][WeekDays[j]] === 1)
                        p.className += " onDay"
                    else 
                        p.className += " offDay"

                    row.appendChild(p)
                }

                table.appendChild(row);
            }

            Schedules.appendChild(table);
            
        });  
        
        StCard.appendChild(Schedules);
    }

    docmain.appendChild(StCard)
}

function OnMorePress() {
    let children = this.parentNode.childNodes;
    let id = children[0].innerHTML;
    id = "\"" + id + "\""
    
    DisplayStudentCard(id)

    mainHistory.saveState();
}



// Back - Forward Functionality

function OnForwBackClick() {
    var bButton = document.getElementById("BackButton");
    var fButton = document.getElementById("ForwardButton");

    bButton.onclick = BackClick;
    fButton.onclick = ForwardClick;
}

function BackClick() { 
    mainHistory.goBack();
    ReassignMainButtons();
}

function ForwardClick() {
    mainHistory.goForward();
    ReassignMainButtons();
}

function ReassignMainButtons() {
    var moreButtons = document.getElementsByClassName("MoreButton");

    for (let i = 0; i < moreButtons.length; i++) {
        moreButtons[i].onclick = OnMorePress;
    }

}



function OnCreateWindow() {
    GenerateBusButtons();
    OnClickBus();
    OnSearchClearStudent();

    docmain = document.getElementsByTagName("main")[0];
    mainHistory = new DOMElementHistory.History(docmain)

    OnForwBackClick();
}




module.exports = {BackClick, ForwardClick};