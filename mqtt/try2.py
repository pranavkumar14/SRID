from csv import reader

def check_user_input(input):
    try:
        # Convert it into integer
        val = int(input)
        print("Input is an integer number. Number = ", val)
    except ValueError:
        try:
            # Convert it into float
            val = float(input)
            print("Input is a float  number. Number = ", val)
        except ValueError:
            print("No.. input is not a number. It's a string")


# open file in read mode
with open('data.csv', 'r') as read_obj:
    # pass the file object to reader() to get the reader object
    csv_reader = reader(read_obj)
    # Iterate over each row in the csv using reader object
    for row in csv_reader:
        # row variable is a list that represents a row in csv
        print(row[0])
        check_user_input(row[0])
