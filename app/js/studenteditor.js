// const ipcRenderer = require("electron").ipcRenderer;
let newSchedules = [];
let deletedSchedules = [];
let CurrentStudent;
let allSchedules = {};
let FormatErrors = [];

let blue = "rgb(30, 87, 153)";
let red = "#be102d";
let grey = "rgb(46, 46, 46)";

function LoadStudents(Student) {
    $(document).ready(function() {
        $("#StudentNameHeader").text(Student.LastName + " " + Student.FirstName + ": " + Student.ClassLevel);
        $("title").text("Student Editor: " + Student.LastName + " " + Student.FirstName);

        let DayParts = ["Morning", "Noon", "Study"]
        DayParts.forEach((dayPart) => {
            LoadSchedules(Student, dayPart);
        });

    });

    CurrentStudent = Student;
}

function LoadSchedules(Student, DayPart) {
    let DayPartSchedules = DayPart + "Schedules";

    for (let i = 0; i < Student[DayPartSchedules].length; i++) {
        
        let Schedule = Student[DayPartSchedules][i];
        LoadOneSchedule(Schedule, DayPartSchedules);
        allSchedules[Schedule.ScheduleID] = Schedule;
    }
                            
    let addbutton = document.createElement("button");
    addbutton.className = "CreateScheduleButton";
    addbutton.onclick = CreateSchedule;
    
    let img = document.createElement("img");
    img.src = "../images/General/plus.png";
    img.className = "CreateScheduleImage";
    addbutton.appendChild(img);

    document.getElementById(DayPartSchedules).appendChild(addbutton);
}

function LoadOneSchedule(Schedule, DayPartSchedules) {

    let DropOrPickup;
    if (Schedule.DayPart === "Morning")
        DropOrPickup = "Pickup";
    else 
        DropOrPickup = "Drop";

    let table = document.createElement("div");
    table.id = Schedule.ScheduleID;
    table.className = "ScheduleTable";

    let closeButton = document.createElement("button");
    closeButton.className = "ScheduleCloseButton"
    closeButton.onclick = DeleteSchedule

    let closeImg = document.createElement("img");
    closeImg.src = "../images/General/x.png";
    closeImg.className = "ScheduleCloseButtonImage"
    closeButton.appendChild(closeImg);

    let topBar = document.createElement("div");
    topBar.className = "ScheduleTopBar";
    topBar.appendChild(closeButton);

    // Road
    SimpleLabelInput(table, "Road", "RoadContent", Schedule.Address.Road);

    // Number
    SimpleLabelInput(table, "Number", "NumberContent", Schedule.Address.Number);

    // ZipCode
    SimpleLabelInput(table, "Zip Code", "ZipCodeContent", Schedule.Address.ZipCode);

    // Municipal
    SimpleLabelInput(table, "Municipal", "MunicipalContent", Schedule.Address.Municipal);

    // BusSchedule
    SimpleLabelInput(table, "Schedule Name", "BusScheduleContent", Schedule.BusSchedule);

    // ScheduleTime
    TimeLabelInput(table, "Schedule Time", "ScheduleTime", Schedule.ScheduleTime, false);

    // ScheduleOrder
    SimpleLabelInput(table, "Schedule Order", "ScheduleOrderContent", Schedule.ScheduleOrder);


    let EmptyTW = Schedule.Early === "00.00" && Schedule.Late === "23.59"
    let EmptyAround = Schedule.Around === "00.00" || !Schedule.Around;

    // Early
    TimeLabelInput(table, "Early " + DropOrPickup, "Early", Schedule.Early, EmptyTW);

    // Late
    TimeLabelInput(table, "Late " + DropOrPickup, "Late", Schedule.Late, EmptyTW);

    // Around
    TimeLabelInput(table, "Around " + DropOrPickup, "Around", Schedule.Around, EmptyAround);

    // Note
    SimpleLabelInput(table, "Notes", "NotesContent", Schedule.Notes, true);

    // Days
    {
        let WeekDays = ['Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday'];
        let Abbr = ["Mon", "Tue", "Wen", "Thu", "Fri"];
        lab = document.createElement("label");
        lab.className = "DaysLabel";
        lab.innerHTML = "Days";
        table.appendChild(lab);

        let days = document.createElement("div");
        days.className ="DaysContent";
        table.appendChild(days);

        for (let j = 0; j < WeekDays.length; j++) {
            let button = document.createElement("button");
            button.className = WeekDays[j] + " DayButton";
            button.innerHTML = Abbr[j];
            if (Schedule.Days[WeekDays[j]] === 1)
                button.className += " OnDay";
            else 
                button.className += " OffDay";

            button.onclick = SwitchDay;
            days.appendChild(button);
        }
    }

    let container = document.createElement("div");
    container.className = "TableClose";
    document.getElementById(DayPartSchedules).appendChild(container);
    container.appendChild(topBar);
    container.appendChild(table)

    $("#" + table.id + " input").change(traceChange);
    $("#" + table.id + " textarea").change(traceChange);
    
    setTimeout(() => {
        table.style.opacity = "1";
        topBar.style.opacity = "1";
        closeButton.style.opacity = "1";
    }, 1);
    
}

function SimpleLabelInput(Parent, Label, InputClass, Value, Textarea=false) {
    let lab = document.createElement("label");
    lab.innerHTML = Label;
    
    let input;

    if (!Textarea)
        input = document.createElement("input");
    else
        input = document.createElement("textarea");
    input.type = "text";
    input.className = InputClass;
    if (Value)
        input.value = Value;

    Parent.appendChild(lab);
    Parent.appendChild(input);
}

function TimeLabelInput(Parent, Label, InputClass, Value, Empty) {
    let lab = document.createElement("label");
    lab.innerHTML = Label;
    
    let hourminute = document.createElement("div");
    hourminute.classList.add(InputClass + "Content", "HourMinuteContent");

    let input = document.createElement("input");
    input.type = "text";
    input.classList.add(InputClass + "HourContent", "HourContent");
    input.placeholder = "Hour";

    if (Value && !Empty)
        input.value = Value.charAt(0) + Value.charAt(1);
    hourminute.appendChild(input);

    let p = document.createElement("p");
    p.className = "HourMinuteSeparator";
    p.innerHTML = ":";
    hourminute.appendChild(p);

    input = document.createElement("input");
    input.type = "text";
    input.classList.add(InputClass + "MinuteContent", "MinuteContent");
    input.placeholder = "Minute";

    if (Value && !Empty)
        input.value = Value.charAt(3) + Value.charAt(4);
    hourminute.appendChild(input);

    Parent.appendChild(lab);
    Parent.appendChild(hourminute);
}




function Save() {
    if (!confirm("Are you sure you want to save?"))
        return;

    const invalidChanged = document.getElementsByClassName("InvalidChanged");

    console.log('invalidChanged', invalidChanged);
    
    if (invalidChanged.length > 0) {
        alert("There are errors to be solved..");
        return;
    }
    
    let DayParts = ["Morning", "Noon", "Study"];

    let SchedulesToSave = {
        New: [],
        Existing: [],
        Deleted: deletedSchedules
    };

    // Loaded Schedules
    for (let j = 0; j < DayParts.length; j++) {
        let dayPart = DayParts[j];

        let DayPartSchedules = dayPart + "Schedules";
        
        for (let i = 0; i < CurrentStudent[DayPartSchedules].length; i++) {

            if ( findInDeleted(CurrentStudent[DayPartSchedules][i].ScheduleID) )
                continue;

            let Schedule = CurrentStudent[DayPartSchedules][i];

            let ScheduleChanges = GetScheduleChanges(Schedule);

            
            if (ScheduleChanges) {
                ScheduleChanges.StudentID = CurrentStudent.ID;
                SchedulesToSave.Existing.push(ScheduleChanges);
            }

            console.log('ScheduleChanges', ScheduleChanges);
        }
    }

    // New Schedules

    for (let i = 0; i < newSchedules.length; i++) {

        if ( findInDeleted(newSchedules[i].ScheduleID) )
            continue;

        let Schedule = newSchedules[i];

        let ScheduleChanges = GetScheduleChanges(Schedule);

        if (ScheduleChanges)
            ScheduleChanges.DayPart = Schedule.DayPart;
            ScheduleChanges.StudentID = CurrentStudent.ID;
            SchedulesToSave.New.push(ScheduleChanges);
        
        console.log('ScheduleChanges', ScheduleChanges);          
            
    }

    
    if (SchedulesToSave.New.length > 0 || SchedulesToSave.Existing.length > 0 || SchedulesToSave.Deleted.length > 0)
        ;// ipcRenderer.send("Save", SchedulesToSave, CurrentStudent);
    else
        alert("Nothing to save..");

}


function DeleteSchedule() {
    deletedSchedules.push( { ScheduleID: this.parentNode.parentNode.childNodes[1].id } );
    this.parentNode.parentNode.childNodes[1].style.opacity = "0";
    this.style.opacity = "0";
    this.parentNode.style.opacity = "0"
    setTimeout(() => {
        this.parentNode.parentNode.parentNode.removeChild(this.parentNode.parentNode);
    }, 300);
    // console.log(deletedSchedules);
}


function SwitchDay() {
    if (this.classList.contains("OnDay")) {
        this.classList.replace("OnDay", "OffDay")
    }
    else if (this.classList.contains("OffDay")) {
        this.classList.replace("OffDay", "OnDay");
    }
}

function CreateSchedule() {
    
    let newSchedule = {
        ScheduleID: "New" + newSchedules.length,
        Address: {
            Road: "",
            Number: "",
            ZipCode: "",
            Municipal: ""
        },
        ScheduleTime: null,
        BusSchedule: null,
        Early: null,
        Late: null,
        Around: null,
        Notes: null,
        Days: {
            Monday: 0,
            Tuesday: 0,
            Wednesday: 0,
            Thursday: 0,
            Friday: 0
        },
        DayPart: this.parentNode.id.replace("Schedules", "")
    }

    newSchedules.push(newSchedule);
    allSchedules[newSchedule.ScheduleID] = newSchedule;

    LoadOneSchedule(newSchedule, this.parentNode.id);

    let addbutton = document.createElement("button");
    addbutton.className = "CreateScheduleButton";
    addbutton.onclick = CreateSchedule;
    let img = document.createElement("img");
    img.src = "../images/General/plus.png";
    img.className = "CreateScheduleImage";
    addbutton.appendChild(img);
    document.getElementById(this.parentNode.id).appendChild(addbutton);

    this.parentNode.removeChild(this);
}

function checkTimeFormat(ContainerName, Hour, Minute) {
    let error = false;
    let errorMessage = "";
    
    if (Hour.length > 2 || Minute.length > 2) {
        error = true;
        errorMessage = ContainerName + ": Invalid Format: \"" + Hour + "." + Minute + "\"";
    }
    
    let intMin = parseInt(Minute);
    let intHour = parseInt(Hour);

    if ( isNaN(intHour) || isNaN(intMin) ) {
        error = true;
        errorMessage = ContainerName + ": Invalid Format: \"" + Hour + "." + Minute + "\"";
    }

    
    if (intMin > 59 || intMin < 0 || intHour > 23 || intHour < 0) {
        error = true;
        errorMessage = ContainerName + ": Invalid Format: \"" + Hour + "." + Minute + "\"";
    }

    if (Hour.length === 1 && !error) Hour = "0" + Hour;
    if (Minute.length === 1 && !error) Minute = "0" + Minute;

    let retval = {
        Hour: Hour,
        Minute: Minute,
        Time: Hour + "." + Minute
    }

    if (error) {
        retval.Error = true;
        retval.ErrorMessage = errorMessage
    }

    return retval;

}

function AddMinutes(StartTime, Offset) {
    let Hour = parseInt(StartTime.charAt(0) + StartTime.charAt(1));
    let Minute = parseInt(StartTime.charAt(3) + StartTime.charAt(4));

    if (Minute + Offset >= 60) {
        Hour += 1;
        Minute = Minute + Offset - 60
    }
    else {
        Minute += Offset;
    }

    Hour = Hour.toString();
    if (Hour.length === 1) Hour = "0" + Hour;

    Minute = Minute.toString();
    if (Minute.length === 1) Minute = "0" + Minute;

    return Hour + "." + Minute;
}

function SubMinutes(StartTime, Offset) {
    let Hour = parseInt(StartTime.charAt(0) + StartTime.charAt(1));
    let Minute = parseInt(StartTime.charAt(3) + StartTime.charAt(4));

    if (Minute - Offset < 0) {
        Hour -= 1;
        Minute = Minute - Offset + 60
    }
    else {
        Minute -= Offset;
    }

    Hour = Hour.toString();
    if (Hour.length === 1) Hour = "0" + Hour;

    Minute = Minute.toString();
    if (Minute.length === 1) Minute = "0" + Minute;

    return Hour + "." + Minute;
}

function findInDeleted(ID) {
    let found = false;
    for (let i = 0; i < deletedSchedules.length; i++) {
        if (deletedSchedules[i].ScheduleID === ID) {
            found = true;
            break;
        }
    }

    return found
}

function GetScheduleChanges(Schedule) {
    let ID = Schedule.ScheduleID;
    let DomSchedule = document.getElementById(ID);
    let ScheduleChanges = {ScheduleID: ID};
    let changedAddress = false;

    let AddressValues = {};

    $("#" + ID + " .ValidChanged").each(function() {

        let dom = $(this).get()[0];

        if (dom.classList.contains("HourMinuteContent") ) {
            let className = dom.classList[0].replace("Content", "");
            if (dom.childNodes[0].value && dom.childNodes[2].value)
                ScheduleChanges[className] = dom.childNodes[0].value + "." + dom.childNodes[2].value;
            else
                ScheduleChanges[className] = null;
        }
        else {
            let className = dom.classList[0].replace("Content", "");
            if (className === "Road" || className === "Number" || className === "ZipCode" || className === "Municipal") {
                changedAddress = true;
                AddressValues[className] = dom.value;
            }
            else
                ScheduleChanges[className] = dom.value;
        }
    })

    if (changedAddress) {
        ScheduleChanges.Address = {};
        ScheduleChanges.Address.Road = AddressValues.Road;
        ScheduleChanges.Address.Number = AddressValues.Number;
        ScheduleChanges.Address.ZipCode = AddressValues.ZipCode;
        ScheduleChanges.Address.Municipal = AddressValues.Municipal;
    }

    // Fix Early & Late according to Around if Around was changed.
    // (If it was not changed that means, Early & Late were fixed beforehand)
    if (ScheduleChanges.hasOwnProperty("Around")) {
        if (!ScheduleChanges.hasOwnProperty("Early") && !ScheduleChanges.hasOwnProperty("Late")) {
            ScheduleChanges.Early = SubMinutes(ScheduleChanges.Around, 7);
            
            DomSchedule.querySelector(".EarlyHourContent").value = ScheduleChanges.Early.charAt(0) + ScheduleChanges.Early.charAt(1);
            DomSchedule.querySelector(".EarlyMinuteContent").value = ScheduleChanges.Early.charAt(3) + ScheduleChanges.Early.charAt(4);

            ScheduleChanges.Late = AddMinutes(ScheduleChanges.Around, 7);

            DomSchedule.querySelector(".LateHourContent").value = ScheduleChanges.Late.charAt(0) + ScheduleChanges.Late.charAt(1);
            DomSchedule.querySelector(".LateMinuteContent").value = ScheduleChanges.Late.charAt(3) + ScheduleChanges.Late.charAt(4);
        }
    }


    let WeekDays = ['Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday'];

    // Always write days
    let DaysChange = false;
    ScheduleChanges.Days = {};
    for (let j = 0; j < WeekDays.length; j++) {

        let DayBit = DomSchedule.querySelector("." + WeekDays[j]).classList.contains("OnDay");

        if (DayBit && Schedule.Days[WeekDays[j]]) {
            ScheduleChanges.Days[WeekDays[j]] = 1;
        }
        else if(!DayBit && !Schedule.Days[WeekDays[j]]) {
            ScheduleChanges.Days[WeekDays[j]] = 0;
        }
        else if (DayBit && !Schedule.Days[WeekDays[j]]) {
            ScheduleChanges.Days[WeekDays[j]] = 1;
            DaysChange = true;
        }
        else if (!DayBit && Schedule.Days[WeekDays[j]]) {
            ScheduleChanges.Days[WeekDays[j]] = 0;
            DaysChange = true;
        }
    }

    
    
    if (Object.keys(ScheduleChanges).length !== 2 || DaysChange) {
        return ScheduleChanges;
    }
    else {
        return null;
    }
}

function traceChange() {
    if (this.classList.contains("HourContent") || this.classList.contains("MinuteContent")) {

        let id = this.parentNode.parentNode.id;
        let changed = false;

        let Hour;
        let Minute;

        if (this.classList.contains("HourContent")) {
            Hour = this;
            Minute = this.parentNode.childNodes[2];
        }
        else {
            Hour = this.parentNode.childNodes[0];
            Minute = this;
        }

        let time = checkTimeFormat(this.parentNode.classList[0].replace("Content", ""), Hour.value, Minute.value);
        Hour.value = time.Hour;
        Minute.value = time.Minute;

        // this returns whole hour (e.g 08.34)
        let Selector = allSchedules[id][ Hour.classList[0].replace("HourContent", "") ];

        if (Selector) {
            if ( (Selector.charAt(0) + Selector.charAt(1) !== Hour.value) ||
                (Selector.charAt(3) + Selector.charAt(4) !== Minute.value) )
            {
                changed = true;
                if (Hour.value === "" && Minute.value === "") {
                    time.Error = false;
                }
            }
        }
        // If selector is empty that values started empty as well
        else {
            if (Hour.value !== "" || Minute.value !== "") {
                changed = true;      
            }
            else {
                time.Error = false;
            }
        }

        if (time.Error) {
            Hour.style.backgroundColor = red;
            Minute.style.backgroundColor = red;
            this.parentNode.title = "Error: \"" + time.ErrorMessage;

            this.parentNode.classList.remove("ValidChanged");
            this.parentNode.classList.add("InvalidChanged");
        }
        else if (changed) {
            Hour.style.backgroundColor = blue;
            Minute.style.backgroundColor = blue;

            this.parentNode.classList.add("ValidChanged");
            this.parentNode.classList.remove("InvalidChanged");

            this.parentNode.removeAttribute("title");
        }
        else {
            Hour.style.backgroundColor = grey;
            Minute.style.backgroundColor = grey;
            this.parentNode.removeAttribute("title");

            this.parentNode.classList.remove("ValidChanged");
            this.parentNode.classList.remove("InvalidChanged");
        }

        
    }
    else {
        let id = this.parentNode.id;
        this.value = this.value.toUpperCase();

        let className = this.className.replace("Content", "")
        className = className.replace("ValidChanged", "");
        className = className.replace("InvalidChanged", "");
        className = className.trim();

        let selector;
        
        if (className === "Road" || className === "Number" || className === "ZipCode" || className === "Municipal") {
            selector = allSchedules[id].Address[className];
        }
        else selector = allSchedules[id][className];

        if (selector !== this.value) {
            if (className === "ScheduleOrder") {
                if (selector !== parseInt(this.value)) {

                    if (this.value && isNaN(parseInt(this.value))) {
                        this.style.backgroundColor = red;

                        this.classList.add("InvalidChanged");
                        this.classList.remove("ValidChanged");

                        this.title = "Error: \"" + this.value.toUpperCase() + "\" is not a number.";
                    }
                    else {
                        this.style.backgroundColor = blue;
                        this.classList.remove("InvalidChanged");
                        this.classList.add("ValidChanged");
                        this.removeAttribute("title");
                    }
                }
                else {
                    this.style.backgroundColor = grey;
                    this.removeAttribute("title");
                    this.classList.remove("InvalidChanged");
                    this.classList.remove("ValidChanged");
                }
            }
            else {
                this.style.backgroundColor = blue;
                this.classList.remove("InvalidChanged");
                this.classList.add("ValidChanged");
            }
        }
        else {
            this.style.backgroundColor = grey;
            this.classList.remove("InvalidChanged");
            this.classList.remove("ValidChanged");
        }

    }
}