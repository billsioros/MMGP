from DBManagement import DBManager as DBM
# from harvesine import harvesine

DBManager = DBM("MMGP_Data.db", "AIzaSyBRGHJf69r2tYhvmpJxdayyXfZorTfHu5g")
# DBManager.InsertDistances(harvesine)
# DBManager.Commit()

sql = " Select a1.FullAddress, a2.FullAddress, d.Distance\
        From Address a1, Address a2, Distance d\
        Where d.AddressID_1 = a1.AddressID and d.AddressID_2 = a2.AddressID"

DBManager.Cursor.execute(sql)

rows = DBManager.Cursor.fetchall()

print len(rows)
