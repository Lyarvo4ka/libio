using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Data.OleDb;
using System.Data.SqlClient;

namespace DataBasesNet
{
    class Program
    {
        static void Main(string[] args)
        {
            int numberTables = 0;

            string connetionString = null;
            SqlConnection connection;
            connetionString = "Data Source=WIN-BD5OT7T92EA\\MSSQL_2008;Initial Catalog=UTP;User ID=sa;Password=Admin123";
            connection = new SqlConnection(connetionString);
            try
            {
                connection.Open();
                Console.WriteLine("OK");

                //string queryString = "USE UTP_rec2 ; SELECT name FROM sys.Tables";
                string select_table = "SELECT * FROM[UTP].[dbo].[_Acc17]";

                SqlCommand command = new SqlCommand(
                     select_table, connection);
                SqlDataReader reader = command.ExecuteReader();
                try
                {
                    while (reader.Read())
                    {
                       // ++numberTables;
                        Console.WriteLine(String.Format("{0}",
                            reader[0]));
                    }
                }
                finally
                {
                    // Always call Close when done reading.
                    reader.Close();
                }

                connection.Close();
            }
            catch (Exception ex)
            {
                Console.WriteLine("FAILED");
            }
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

