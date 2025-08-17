# Railway Management System (C Project)

This is a simple console-based Railway Reservation System written in C. It allows users to manage train details, book and cancel tickets, and view booked tickets using file handling for persistent storage.

## Features

- Add, view, and delete train records
- Book tickets for available trains
- Cancel booked tickets
- View all booked tickets
- Data stored persistently in binary files (`trains.dat`, `tickets.dat`)

## Files

- `railway_management.c` — Main source code file
- `trains.dat` — Binary file storing train records (created automatically)
- `tickets.dat` — Binary file storing passenger ticket records (created automatically)

## How to Compile and Run

1. **Clone the repository:**
   ```bash
   git clone https://github.com/TechajayBT/RailwayManagementSystem.git
   cd RailwayManagementSystem
   ```

2. **Compile the code:**
   ```bash
   gcc railway_management.c -o railway_management
   ```

3. **Run the program:**
   ```bash
   ./railway_management
   ```

## Usage

When you run the program, you will see a menu:

```
===== Railway Reservation System =====
1. Add Train
2. View Trains
3. Delete Train
4. Book Ticket
5. Cancel Ticket
6. View Booked Tickets
7. Exit
Enter your choice:
```

- Follow the prompts to perform operations.
- All data is saved in `.dat` files in the current directory.

## Sample Data Structure

**Train:**
- Train No
- Name
- Source
- Destination
- Seats (available)
- Fare

**Passenger Ticket:**
- Ticket ID
- Name
- Age
- Train No
- Seat No

## Notes

- Make sure you have write permission in the folder to create and modify `.dat` files.
- The program uses simple file-based storage (no external database required).
- To reset the system, delete the `.dat` files.

## License

This project is open-source and available under the MIT License.
