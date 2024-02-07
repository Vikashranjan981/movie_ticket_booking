#include <iostream>
#include <mysql/mysql.h>
#include <sstream>
#include <cstdlib> // for system("cls")
using namespace std;

const char *HOST = "localhost";
const char *USER = "root";
const char *PW = "your_password";
const char *DB = "mydb";

class Seats
{
private:
    int Seat[5][10];

public:
    Seats()
    {
        for (int i = 0; i < 5; i++)
        {
            for (int j = 0; j < 10; j++)
            {
                Seat[i][j] = 1;
            }
        }
    }

    int getSeatStatus(int row, int seatNumber)
    {
        if (row < 1 || row > 5 || seatNumber < 1 || seatNumber > 10)
        {
            return -1;
        }
        return Seat[row - 1][seatNumber - 1];
    }

    void reserveSeat(int row, int seatNumber)
    {
        if (row < 1 || row > 5 || seatNumber < 1 || seatNumber > 10)
        {
            return;
        }
        Seat[row - 1][seatNumber - 1] = 0;
    }

    void display()
    {
        system("cls");
        cout << " ";
        for (int i = 0; i < 10; i++)
        {
            cout << " " << i + 1;
        }
        cout << endl;

        for (int row = 0; row < 5; row++)
        {
            cout << row + 1 << " ";
            for (int col = 0; col < 10; col++)
            {
                if (Seat[row][col] == 1)
                {
                    cout << "- ";
                }
                else
                {
                    cout << "X ";
                }
            }
            cout << "|" << endl;
        }
        cout << "-----------------------" << endl;
    }

    void updateFromDB(MYSQL *conn)
    {
        string query = "SELECT RowNumber, SeatNumber, Seat FROM Ticket";
        if (mysql_query(conn, query.c_str()))
        {
            cout << "Error: " << mysql_error(conn) << endl;
            return;
        }

        MYSQL_RES *result;
        result = mysql_store_result(conn);
        if (!result)
        {
            cout << "Error: " << mysql_error(conn) << endl;
            return;
        }

        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result)))
        {
            int rowNumber = atoi(row[0]);
            int seatNumber = atoi(row[1]);
            int seatStatus = atoi(row[2]);
            Seat[rowNumber - 1][seatNumber - 1] = seatStatus;
        }
        mysql_free_result(result);
    }
};

int main()
{
    Seats s;
    MYSQL *conn;
    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, HOST, USER, PW, DB, 3306, NULL, 0))
    {
        cout << "Error: " << mysql_error(conn) << endl;
        return 1;
    }
    else
    {
        cout << "Logged In Database!" << endl;
    }

    if (mysql_query(conn, "CREATE TABLE IF NOT EXISTS Ticket (RowNumber INT, SeatNumber INT, Seat INT)"))
    {
        cout << "Error: " << mysql_error(conn) << endl;
        return 1;
    }

    for (int row = 1; row <= 5; row++)
    {
        for (int seatNumber = 1; seatNumber <= 10; seatNumber++)
        {
            stringstream ss;
            ss << "INSERT INTO Ticket (RowNumber,SeatNumber,Seat) "
               << "SELECT '" << row << "', '" << seatNumber << "','1' "
               << "WHERE NOT EXISTS (SELECT * FROM Ticket WHERE RowNumber = '" << row << "' AND SeatNumber = '" << seatNumber << "')";
            string insertQuery = ss.str();
            if (mysql_query(conn, insertQuery.c_str()))
            {
                cout << "Error: " << mysql_error(conn);
                return 1;
            }
        }
    }

    bool exitFlag = false;
    while (!exitFlag)
    {
        s.updateFromDB(conn);
        s.display();

        int choice;
        cout << "Welcome To Movie Ticket Booking System" << endl;
        cout << "******************************************" << endl;
        cout << "1. Reserve A Ticket" << endl;
        cout << "2. Exit" << endl;
        cout << "Enter Your Choice: ";
        cin >> choice;

        switch (choice)
        {
        case 1:
        {
            int row, col;
            cout << "Enter Row (1-5): ";
            cin >> row;
            cout << "Enter Seat Number (1-10): ";
            cin >> col;

            if (row < 1 || row > 5 || col < 1 || col > 10)
            {
                cout << "Invalid Row or Seat Number!" << endl;
                break;
            }

            int seatStatus = s.getSeatStatus(row, col);
            if (seatStatus == -1)
            {
                cout << "Invalid Row or Seat Number!" << endl;
                break;
            }

            if (seatStatus == 0)
            {
                cout << "Sorry: Seat is already reserved!" << endl;
                break;
            }

            s.reserveSeat(row, col);
            stringstream ss;
            ss << "UPDATE Ticket SET Seat = 0 WHERE RowNumber = " << row << " AND SeatNumber =" << col;
            string update = ss.str();
            if (mysql_query(conn, update.c_str()))
            {
                cout << "Error: " << mysql_error(conn) << endl;
                return 1;
            }
            else
            {
                cout << "Seat Is Reserved Successfully in Row " << row << " and Seat Number " << col << endl;
            }
            break;
        }
        case 2:
            exitFlag = true;
            cout << "Good Luck!" << endl;
            break;
        default:
            cout << "Invalid Input" << endl;
            break;
        }
    }

    mysql_close(conn);
    return 0;
}
