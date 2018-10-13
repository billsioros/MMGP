// const ipcRenderer = require("electron").ipcRenderer;

let CurrentStudent;

function LoadStudents(Student) {
    document.getElementById("StudentNameHeader").innerHTML = Student.LastName + " " + Student.FirstName + ": " + Student.ClassLevel;
    document.getElementsByTagName("title")[0].innerHTML = "Student Editor: " + Student.LastName + " " + Student.FirstName;

    let DayParts = ["Morning", "Noon", "Study"]
    DayParts.forEach((dayPart) => {
        LoadSchedules(Student, dayPart);
    });

    CurrentStudent = Student;
}

function LoadSchedules(Student, DayPart) {
    let DayPartSchedules = DayPart + "Schedules";

    for (let i = 0; i < Student[DayPartSchedules].length; i++) {
        
        let Schedule = Student[DayPartSchedules][i];

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

        // Address
        let lab = document.createElement("label");
        lab.className = "AddressLabel";
        lab.innerHTML = "Address";
        table.appendChild(lab);
        let input = document.createElement("input");
        input.type = "text";
        input.className = "AddressContent";
        input.value = Schedule.Address.FullAddress;
        table.appendChild(input)

        // BusSchedule
        lab = document.createElement("label");
        lab.className = "BusScheduleLabel";
        lab.innerHTML = "Schedule Name";
        table.appendChild(lab);
        input = document.createElement("input");
        input.type = "text";
        input.className = "BusScheduleContent";
        input.value = Schedule.BusSchedule;
        table.appendChild(input)

        // ScheduleTime
        lab = document.createElement("label");
        lab.className = "ScheduleTimeLabel";
        lab.innerHTML = "Schedule Time";
        table.appendChild(lab);
        input = document.createElement("input");
        input.type = "text";
        input.className = "ScheduleTimeContent";
        input.value = Schedule.ScheduleTime;
        table.appendChild(input)

        // Early
        lab = document.createElement("label");
        lab.className = "EarlyLabel";
        lab.innerHTML = "Early" + " " + DropOrPickup;
        table.appendChild(lab);
        input = document.createElement("input");
        input.type = "text";
        input.className = "EarlyContent";
        input.value = Schedule.Early;
        table.appendChild(input)

        // Late
        lab = document.createElement("label");
        lab.className = "LateLabel";
        lab.innerHTML = "Late" + " " + DropOrPickup;
        table.appendChild(lab);
        input = document.createElement("input");
        input.type = "text";
        input.className = "LateContent";
        input.value = Schedule.Late;
        table.appendChild(input)

        // Around
        lab = document.createElement("label");
        lab.className = "AroundLabel";
        lab.innerHTML = DropOrPickup + " " + "Around";
        table.appendChild(lab);
        input = document.createElement("input");
        input.type = "text";
        input.className = "AroundContent";
        input.value = Schedule.Around;
        table.appendChild(input)

        // Note
        lab = document.createElement("label");
        lab.className = "NotesLabel";
        lab.innerHTML = "Notes";
        table.appendChild(lab);
        input = document.createElement("input");
        input.type = "text";
        input.className = "NotesContent";
        input.value = Schedule.Notes;
        table.appendChild(input)

        // Days
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

        // div.appendChild(table);

        document.getElementById(DayPartSchedules).appendChild(table);
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

function Save() {
    if (!confirm("Are you sure you want to save?"))
        return;
    
    let DayParts = ["Morning", "Noon", "Study"]
    let SchedulesToSave = [];
    for (let j = 0; j < DayParts.length; j++) {
        let dayPart = DayParts[j];

        let DayPartSchedules = dayPart + "Schedules";

        for (let i = 0; i < CurrentStudent[DayPartSchedules].length; i++) {
            let Schedule = CurrentStudent[DayPartSchedules][i];

            let DomSchedule = document.getElementById(Schedule.ScheduleID);

            Schedule.Address.FullAddress = DomSchedule.querySelector(".AddressContent").value;
            Schedule.BusSchedule = DomSchedule.querySelector(".BusScheduleContent").value;
            Schedule.ScheduleTime = DomSchedule.querySelector(".ScheduleTimeContent").value;
            Schedule.Early = DomSchedule.querySelector(".EarlyContent").value;
            Schedule.Late = DomSchedule.querySelector(".LateContent").value;
            Schedule.Around = DomSchedule.querySelector(".AroundContent").value;
            Schedule.Notes = DomSchedule.querySelector(".NotesContent").value;
            
            let WeekDays = ['Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday'];

            for (let j = 0; j < WeekDays.length; j++) {
                if (DomSchedule.querySelector("." + WeekDays[j]).classList.contains("OnDay"))
                    Schedule.Days[WeekDays[j]] = 1;
                else
                    Schedule.Days[WeekDays[j]] = 0;       
            }
        }
    }

}