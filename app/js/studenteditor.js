// const ipcRenderer = require("electron").ipcRenderer;
let newSchedules = [];
let CurrentStudent;

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
    // let table = document.createElement("div");
    // table.className = "ScheduleTableRow";

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


    let EmptyTW = Schedule.Early === "00.00" && Schedule.Late === "23.59"
    let EmptyAround = Schedule.Around === "00.00" || !Schedule.Around;

    // Early
    TimeLabelInput(table, "Early " + DropOrPickup, "Early", Schedule.Early, true);

    // Late
    TimeLabelInput(table, "Late " + DropOrPickup, "Late", Schedule.Late, true);

    // Around
    TimeLabelInput(table, "Around " + DropOrPickup, "Around", Schedule.Around, true);

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

    document.getElementById(DayPartSchedules).appendChild(table);

    setTimeout(() => {
        table.style.opacity = "1";
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
    
    let DayParts = ["Morning", "Noon", "Study"];

    let SchedulesToSave = {
        New: [],
        Existing: []
    };

    let FormatErrors = [];

    // Loaded Schedules
    for (let j = 0; j < DayParts.length; j++) {
        let dayPart = DayParts[j];

        let DayPartSchedules = dayPart + "Schedules";
        
        for (let i = 0; i < CurrentStudent[DayPartSchedules].length; i++) {

            let Schedule = CurrentStudent[DayPartSchedules][i];
            let DomSchedule = document.getElementById(Schedule.ScheduleID);

            let ScheduleChanges = GetScheduleChanges(Schedule, DomSchedule);
            // ScheduleChanges.ScheduleID = Schedule.ScheduleID;

            console.log('ScheduleChanges', ScheduleChanges);
            SchedulesToSave.Existing.push(ScheduleChanges);
        }
    }

    // New Schedules

    for (let i = 0; i < newSchedules.length; i++) {
        let Schedule = newSchedules[i];
        let DomSchedule = document.getElementById("New" + i)

        let ScheduleChanges = GetScheduleChanges(Schedule, DomSchedule);

        console.log('ScheduleChanges', ScheduleChanges);
            SchedulesToSave.New.push(ScheduleChanges);
    }

    if (FormatErrors.length !== 0) {
        let fullError = ""
        for (let i = 0; i < FormatErrors.length; i++) {
            fullError += ("\n" + FormatErrors[i]);
            console.error(FormatErrors[i]);
        }
        alert(fullError);
    }

}

function GetScheduleChanges(Schedule, DomSchedule) {

    let ScheduleChanges = {ScheduleID: Schedule.ScheduleID};

    let change = DomSchedule.querySelector(".RoadContent");
    if (Schedule.Address.Road !== change.value) {
        if (!ScheduleChanges.hasOwnProperty("Address"))
            ScheduleChanges.Address = {};
        change.value = change.value;
        ScheduleChanges.Address.Road = change.value;
    }

    change = DomSchedule.querySelector(".NumberContent");
    if (Schedule.Address.Number !== change.value) {
        if (!ScheduleChanges.hasOwnProperty("Address"))
            ScheduleChanges.Address = {};
        change.value = change.value;
        ScheduleChanges.Address.Number = change.value;
    }

    change = DomSchedule.querySelector(".ZipCodeContent");
    if (Schedule.Address.ZipCode !== change.value) {
        if (!ScheduleChanges.hasOwnProperty("Address"))
            ScheduleChanges.Address = {};
        change.value = change.value;
        ScheduleChanges.Address.ZipCode = change.value;
    }

    change = DomSchedule.querySelector(".MunicipalContent");
    if (Schedule.Address.Municipal !== change.value) {
        if (!ScheduleChanges.hasOwnProperty("Address"))
            ScheduleChanges.Address = {};
        change.value = change.value;
        ScheduleChanges.Address.Municipal = change.value;
    }


    change = DomSchedule.querySelector(".BusScheduleContent");
    if (Schedule.BusSchedule !== change.value && change.value) {
        change.value = change.value;
        ScheduleChanges.BusSchedule = change.value;
    }


    let TimeContainers = [
        "ScheduleTime",
        "Early",
        "Late",
        "Around"
    ];

    for (let i = 0; i < TimeContainers.length; i++) {
        let changeHour = DomSchedule.querySelector("." + TimeContainers[i] + "HourContent")
        let changeMinute = DomSchedule.querySelector("." + TimeContainers[i] + "MinuteContent")

        if (Schedule[TimeContainers[i]] !== (changeHour.value + "." + changeMinute.value) && (changeHour.value || changeMinute.value) ) {
            let time = checkTimeFormat(TimeContainers[i], changeHour.value, changeMinute.value);

            if (time.Error) {
                FormatErrors.push(time.ErrorMessage);
            }
            else {
                ScheduleChanges[TimeContainers[i]] = time.Time;
                changeHour.value = time.Hour;
                changeMinute.value = time.Minute;
            }
        }
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

    change = DomSchedule.querySelector(".NotesContent");
    if (Schedule.Notes !== change.value && change.value) {
        change.value = change.value;
        ScheduleChanges.Notes = change.value;
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
            Road: null,
            Number: null,
            ZipCode: null,
            Municipal: null
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

    if (Hour.length === 1) Hour = "0" + Hour;
    if (Minute.length === 1) Minute = "0" + Minute;

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