# MMGP_Data.db Instructions

## General Info
The database is a sqlite3 database. That means that some more advanced queries are not supported.

## Clustering

### Students
We need **StudentID, AddressID, Monday, Tuesday, Wednesday, Thursday, Friday, DayPart** for each student

To get those fields from the database:


```sql
Select StudentID, AddressID, Monday, Tuesday, Wednesday, Thursday, Friday, DayPart
From Student
```

Since the same students may appear in different       **DayParts**, it is highly advised (it is **mandatory**) to do different clusterings for each different **DayPart**. So, we need to grab the students for specific DayParts:

```sql
Select StudentID, AddressID, Monday, Tuesday, Wednesday, Thursday, Friday, DayPart
From Student
Where DayPart = <insert_DayPart>
```

Current valid DayParts are: *"Morning", "Noon", "Study"* **With** the quotation marks.

### Distances
In order to get the distance between two Students, a.k.a between two Addresses, you will need to search the distance tables. There are currently **3** distance tables, one for each valid DayPart.

Current valid Distance Tables are: *"MorningDistance", "NoonDistance", "StudyDistance"*.

So, to get the distance between 2 addresses:

```sql
Select Distance, Duration
From <insert_Distance_Table>
Where AddressID_1 = <first_address> 
      and AddressID_2 = <second_address>  
```
*Disclaimer: Distance and Duration fields are integers (for now)

### Tips for specific tables:
* If you have the **DayPart** you want to do the clustering with, you could concatenate that **DayPart** with the string *"distance"*, and you will have the wanted **Distance Table**
* More to add later...

If you want to get every distance from one address to all the others you could do:

```sql
Select Distance, Duration
From <insert_Distance_Table>
Where AddressID_1 = <insert_address>
```
For a specific destination address to all origin addresses:

```sql
Select Distance, Duration
From <insert_Distance_Table>
Where AddressID_2 = <insert_address>
```

## Dispatching

### Buses

In order to get all available vehicles simply:

```sql
Select BusID, Number, Capacity
From Bus
```
*Disclaimer: Number is not mandatory, but it could be useful for exporting the data after the job is done
