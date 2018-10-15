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
        let hourminute = document.createElement("div");
        hourminute.className = "ScheduleTimeContent HourMinuteContent";

        input = document.createElement("input");
        input.type = "text";
        input.className = "ScheduleHourContent HourContent";
        input.placeholder = "Hour";
        if (Schedule.ScheduleTime)
            input.value = Schedule.ScheduleTime.charAt(0) + Schedule.ScheduleTime.charAt(1);
        hourminute.appendChild(input);
        input = document.createElement("p");
        input.className = "HourMinuteSeparator";
        input.innerHTML = ":";
        hourminute.appendChild(input);

        input = document.createElement("input");
        input.type = "text";
        input.className = "ScheduleMinuteContent MinuteContent";
        input.placeholder = "Minute";
        if (Schedule.ScheduleTime)
            input.value = Schedule.ScheduleTime.charAt(3) + Schedule.ScheduleTime.charAt(4);
        hourminute.appendChild(input);

        table.appendChild(hourminute);

        // Early
        lab = document.createElement("label");
        lab.className = "EarlyLabel";
        lab.innerHTML = "Early " + DropOrPickup;
        table.appendChild(lab);
        hourminute = document.createElement("div");
        hourminute.className = "EarlyContent HourMinuteContent";

        input = document.createElement("input");
        input.type = "text";
        input.className = "EarlyHourContent HourContent";
        input.placeholder = "Hour";
        if (Schedule.Early)
            input.value = Schedule.Early.charAt(0) + Schedule.Early.charAt(1);
        hourminute.appendChild(input);
        input = document.createElement("p");
        input.className = "HourMinuteSeparator";    
        input.innerHTML = ":";
        hourminute.appendChild(input);

        input = document.createElement("input");
        input.type = "text";
        input.className = "EarlyMinuteContent MinuteContent";
        input.placeholder = "Minute";
        if (Schedule.Early)
            input.value = Schedule.Early.charAt(3) + Schedule.Early.charAt(4);
        hourminute.appendChild(input);

        table.appendChild(hourminute);

        // Late
        lab = document.createElement("label");
        lab.className = "LateLabel";
        lab.innerHTML = "Late " + DropOrPickup;
        table.appendChild(lab);
        hourminute = document.createElement("div");
        hourminute.className = "LateContent HourMinuteContent";

        input = document.createElement("input");
        input.type = "text";
        input.className = "LateHourContent HourContent";
        input.placeholder = "Hour";
        if (Schedule.Late)
            input.value = Schedule.Late.charAt(0) + Schedule.Late.charAt(1);
        hourminute.appendChild(input);
        input = document.createElement("p");
        input.className = "HourMinuteSeparator";
        input.innerHTML = ":";
        hourminute.appendChild(input);

        input = document.createElement("input");
        input.type = "text";
        input.className = "LateMinuteContent MinuteContent";
        input.placeholder = "Minute";
        if (Schedule.Late)
            input.value = Schedule.Late.charAt(3) + Schedule.Late.charAt(4);
        hourminute.appendChild(input);

        table.appendChild(hourminute);

        // Around
        lab = document.createElement("label");
        lab.className = "AroundLabel";
        lab.innerHTML = "Around " + DropOrPickup;
        table.appendChild(lab);
        hourminute = document.createElement("div");
        hourminute.className = "AroundContent HourMinuteContent";

        input = document.createElement("input");
        input.type = "text";
        input.className = "AroundHourContent HourContent";
        input.placeholder = "Hour";
        if (Schedule.Around)
            input.value = Schedule.Around.charAt(0) + Schedule.Around.charAt(1);
        hourminute.appendChild(input);
        input = document.createElement("p");
        input.className = "HourMinuteSeparator";
        input.innerHTML = ":";
        hourminute.appendChild(input);

        input = document.createElement("input");
        input.type = "text";
        input.className = "AroundMinuteContent MinuteContent";
        input.placeholder = "Minute";
        if (Schedule.Around)
            input.value = Schedule.Around.charAt(3) + Schedule.Around.charAt(4);
        hourminute.appendChild(input);

        table.appendChild(hourminute);

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
    let FormatErrors = [];

    for (let j = 0; j < DayParts.length; j++) {
        let dayPart = DayParts[j];

        let DayPartSchedules = dayPart + "Schedules";
        
        for (let i = 0; i < CurrentStudent[DayPartSchedules].length; i++) {

            let Schedule = CurrentStudent[DayPartSchedules][i];
            let ScheduleChanges = {ScheduleID: Schedule.ScheduleID};
            let DomSchedule = document.getElementById(Schedule.ScheduleID);

            let change = DomSchedule.querySelector(".AddressContent").value;
            if (Schedule.Address.FullAddress !== change) {
                ScheduleChanges.Address = {};
                ScheduleChanges.Address.FullAddress = change;
            }

            change = DomSchedule.querySelector(".BusScheduleContent").value;
            if (Schedule.BusSchedule !== change && change) {
                ScheduleChanges.BusSchedule = change;
            }

            change = DomSchedule.querySelector(".ScheduleTimeContent").value;
            if (Schedule.ScheduleTime !== change && change) {
                let error = false;
                let errorMessage = "";
                
                if (change.length !== 3 && change.length !== 4) {
                    error = true;
                    errorMessage = "Schedule Time: Invalid Format: \"" + change + "\"";
                }

                if ( isNaN(parseInt(change[0])) ) {
                    error = true;
                    errorMessage = "Schedule Time: Invalid Format: \"" + change + "\"";
                }
                else {
                    if ( isNaN(parseInt(change[1])) ) {
                        if (!(change[2] === ":" || change[2] === "." || change[2] === ",")) {
                            error = true;
                            errorMessage = "Schedule Time: Invalid Format: \"" + change + "\"";
                        }
                        else {
                            if ( isNaN(parseInt(change[3])) || isNaN(parseInt(change[3])) ) {
                                error = true;
                                errorMessage = "Schedule Time: Invalid Format: \"" + change + "\"";
                            }
                        }
                    }
                    else {
                        if (!(change[2] === ":" || change[2] === "." || change[2] === ",")) {
                            error = true;
                            errorMessage = "Schedule Time: Invalid Format: \"" + change + "\"";
                        }
                        else {
                            if ( isNaN(parseInt(change[3])) || isNaN(parseInt(change[3])) ) {
                                error = true;
                                errorMessage = "Schedule Time: Invalid Format: \"" + change + "\"";
                            }
                        }
                    }
                }

                if (error)
                    FormatErrors.push(errorMessage);
                else
                    ScheduleChanges.ScheduleTime = change;          
            }
            
            change = DomSchedule.querySelector(".EarlyContent").value;
            if (Schedule.Early !== change && change) {
                ScheduleChanges.Early = change;
            }

            change = DomSchedule.querySelector(".LateContent").value;
            if (Schedule.Late !== change && change) {
                ScheduleChanges.Late = change;
            }

            change = DomSchedule.querySelector(".AroundContent").value;
            if (Schedule.Around !== change && change) {
                ScheduleChanges.Around = change;
            }

            change = DomSchedule.querySelector(".NotesContent").value;
            if (Schedule.Notes !== change && change) {
                ScheduleChanges.Notes = change;
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
                SchedulesToSave.push(ScheduleChanges);

                console.log(ScheduleChanges);
            }
        }
    }

}