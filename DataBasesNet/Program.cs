using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Data.OleDb;
using System.Data;
using System.Data.SqlClient;
using System.IO;

namespace DataBasesNet
{
    class SqlDataBases
    {
        private List<string> goodTables_;
        private List<string> badTables_;
        private List<string> allTables_;

        SqlConnection connection_;


        public SqlDataBases()
        {
            goodTables_ = new List<string> { };
            badTables_ = new List<string> { };
            allTables_ = new List<string> { };
            //connection = new SqlConnection();
        }

        public string CreateConnectionString(string serverName , string dbName , string userName , string password )
        {
            return "Data Source=" + serverName + ";Initial Catalog=" + dbName + "; User ID=" + userName + ";Password=" + password;
        }
        public void CreateConnection(string connectionString)
        {
            connection_ = new SqlConnection(connectionString); 
        }

        public bool TestTable(SqlConnection connection , string tableName)
        {
            bool testResult = false;
            string select_table = "SELECT * FROM[Don83].[dbo].[" + tableName + "]";

            Console.Write(tableName);

            SqlCommand command = new SqlCommand( select_table, connection);
            SqlDataReader reader = command.ExecuteReader();


            try
            {

                while (reader.Read())
                {
                    break;
                }
                testResult = true;
                Console.WriteLine(" opened OK");
            }
            catch (Exception ex)
            {
                Console.WriteLine(" opened with error {0}" , ex.ToString());

            }
            finally
            {
                // Always call Close when done reading.
                reader.Close();
            }
            return testResult;

        }

        public void TestTables()
        {
            string connetionString = null;
            try
            {
                SqlConnection connection;
                connetionString = "Data Source=WIN-BD5OT7T92EA\\MSSQL_2008;Initial Catalog=Don83;User ID=sa;Password=Admin123";
                connection = new SqlConnection(connetionString);
                connection.Open();

                allTables_ = GetAllTables(connection , "Don83");
                connection.Close();

                foreach (string tableName in allTables_)
                {
                    connection.Open();
                    bool bResult = false;
                    try
                    {
                        bResult = TestTable(connection, tableName);
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine(" opened with error {0}" , ex.ToString());
                    }
                    if (bResult)
                        goodTables_.Add(tableName);
                    else
                        badTables_.Add(tableName);


                    connection.Close();
                }



                connection.Close();
            }
            catch (Exception ex)
            {
                Console.WriteLine("FAILED {0}", ex.ToString());
            }

            TextWriter goodWriter = new StreamWriter("goodTables.txt");
            foreach (String s in goodTables_)
                goodWriter.WriteLine(s);
            goodWriter.Close();

            TextWriter badWriter = new StreamWriter("badTables.txt");
            foreach (String s in badTables_)
                badWriter.WriteLine(s);
            badWriter.Close();



        }

        public List<string> GetAllTables(SqlConnection connection, string dbName)
        {
            List<string> list_tables = new List<string> { };

            string getAllTablesString = "USE " + dbName + "; SELECT name FROM sys.Tables";

            SqlCommand command = new SqlCommand(getAllTablesString, connection);
            SqlDataReader reader = command.ExecuteReader();
            try
            {
                while (reader.Read())
                {
                    string new_table = reader[0].ToString();
                    list_tables.Add(new_table);
                }
            }
            finally
            {
                // Always call Close when done reading.
                reader.Close();
            }
            return list_tables;
        }

        public class ColumnType
        {
            public string column_name_ ;
            public string type_name_;
            public string sql_type_name_;

            //public ColumnType(string column , string type)
            //{
            //    column_name_ = column;
            //    type_name_ = type;

            //}
            //public string FieldType => $"{column_name_} {type_name_}";
        }


        public List<ColumnType> GetTableColunmType(string dbName, string tableName)
        {

            List<ColumnType> listColumType = new List<ColumnType> { };
            SqlConnection connection;
            string connetionString = "Data Source=WIN-BD5OT7T92EA\\MSSQL_2008;Initial Catalog=" + dbName + "; User ID=sa;Password=Admin123";
            connection = new SqlConnection(connetionString);
            DataTable schemaTable;
            SqlCommand cmd = new SqlCommand();
            try
            {
                connection.Open();
                /*
                 select COLUMN_NAME , DATA_TYPE
               from INFORMATION_SCHEMA.COLUMNS
               where TABLE_NAME='_Document211'
                            */

                SqlDataReader myReader;
                 
                //Retrieve records from the Employees table into a DataReader.
                cmd.Connection = connection;    //
                cmd.CommandText = "select * from [" + dbName +"].[dbo].["+ tableName + "]";
                myReader = cmd.ExecuteReader(CommandBehavior.SchemaOnly);



                //Retrieve column schema into a DataTable.
                schemaTable = myReader.GetSchemaTable();

                string ColumnNameText = "ColumnName";
                string DataTypeText = "DataType";
                string DataTypeNameText = "DataTypeName";

                //For each field in the table...
                foreach (DataRow myField in schemaTable.Rows)
                {
                    ColumnType fieldType = new ColumnType();            
                    foreach (DataColumn myProperty in schemaTable.Columns)
                    {
                        if (myProperty.ColumnName == ColumnNameText)
                        {
                            fieldType.column_name_ = myField[myProperty].ToString();
                        }
                        else if (myProperty.ColumnName == DataTypeText)
                        {
                            fieldType.type_name_ = myField[myProperty].ToString();
                        }
                        else if (myProperty.ColumnName == DataTypeNameText)
                        {
                            fieldType.sql_type_name_ = myField[myProperty].ToString();
                        }
                        //Console.WriteLine(myProperty.ColumnName + " = " + myField[myProperty].ToString());
                    }
                    listColumType.Add(fieldType);
//                    Console.ReadLine();
                }

                foreach (var columType in listColumType)
                {
                    Console.WriteLine(columType.column_name_ + " = " + columType.sql_type_name_);
                }
                connection.Close();
            }
            catch(Exception ex)
            {
                Console.WriteLine("Error connect to database {0}" , ex.ToString());
            }

            return listColumType;
        }

        public bool UpdateNullsIfExist(List<ColumnType> columnTypes, string tableName)
        {
            bool bUpated = false;
            string stringTypeName = "System.String";
            foreach (var columType in columnTypes)
            {
                if (columType.type_name_ == stringTypeName)
                {

                    try
                    {
                        connection_.Open();

                        string updatedString = "UPDATE " + tableName + " SET " + columType.column_name_ + " = '' WHERE " + columType.column_name_ + " IS NULL";

                        SqlCommand cmdUpdate = new SqlCommand(updatedString, connection_);
                        int rows = cmdUpdate.ExecuteNonQuery();
                        if (rows > 0)
                            bUpated = true;
                        Console.WriteLine("Number rows updated = {0}" , rows);
                        if (columType.sql_type_name_ == "ntext")
                        {
                            string modifyTable = "ALTER TABLE " + tableName + " ALTER COLUMN " + columType.column_name_ + " " + columType.sql_type_name_ + " NOT NULL";
                            SqlCommand cmdModify = new SqlCommand(modifyTable, connection_);
                            rows = cmdModify.ExecuteNonQuery();
                            Console.WriteLine("Number rows modified = {0}", rows);
                        }

                        connection_.Close();

                    }
                    catch(Exception ex)
                    {
                        Console.WriteLine("Error connect to database {0}", ex.ToString());
                    }
                    finally
                    {
                        connection_.Close();
                    }

                    /*
                     UPDATE Table_1 SET bbb='' WHERE bbb IS NULL
                     ALTER TABLE Table_1 ALTER COLUMN bbb real NOT NULL
*/
                }
            }


            return bUpated;
        }


    }

    class Program
    {



        static void Main(string[] args)
        {
            string[] Tables = System.IO.File.ReadAllLines(@"tableToUpdate.txt");

            foreach (var tableName in Tables)
            {

                SqlDataBases sql_bases = new SqlDataBases();
                var col_types = sql_bases.GetTableColunmType("utp_new", tableName);

                SqlDataBases sql_updater = new SqlDataBases();
                //Data Source=WIN-BD5OT7T92EA\\MSSQL_2008;Initial Catalog=utp_new;User ID=sa;Password=Admin123";
                var connString = sql_bases.CreateConnectionString("WIN-BD5OT7T92EA\\MSSQL_2008", "utp_new", "sa", "Admin123");
                sql_updater.CreateConnection(connString);

                sql_updater.UpdateNullsIfExist(col_types, tableName);

            }


            Console.ReadLine();
        }


    }

}

/*
  string queryString = 
        "SELECT OrderID, CustomerID FROM dbo.Orders;";
    using (SqlConnection connection = new SqlConnection(
               connectionString))
    {
        SqlCommand command = new SqlCommand(
            queryString, connection);
        connection.Open();
        SqlDataReader reader = command.ExecuteReader();
        try
        {
            while (reader.Read())
            {
                Console.WriteLine(String.Format("{0}, {1}",
                    reader[0], reader[1]));
            }
        }
        finally
        {
            // Always call Close when done reading.
            reader.Close();
        }
    }
     */

