from DBmanagement import DBManager as DBM

sql = 'Update Schedule Set Early = "00.00", Late = "23:59"'

Database = "D:\\Users\\Giannis\\Desktop\\BusRouting\\MMGP\\app\\html/../../data/MMGP_data.db"

manager = DBM(Database)

Rows = manager.Execute(sql)

manager.Commit()