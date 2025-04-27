
# install pandas if necessary with
# pip install pandas==1.3.3
# or
# sudo apt install python3-pandas


import pandas as pd

# Define the filenames as variables
file1 = 'fpu_opcode_68020.txt'
file2 = 'fpu_opcode_68040.txt'
file3 = 'fpu_opcode_68060.txt'
file4 = 'fpu_opcode_68020-60.txt'


suffix1 = '_68020'
suffix2 = '_68040'
suffix3 = '_68060'
suffix4 = '_6820-60'


# Load the CSV files into DataFrames
df1 = pd.read_csv(file1)
df2 = pd.read_csv(file2)
df3 = pd.read_csv(file3)
df4 = pd.read_csv(file4)

# Print the first few rows of each DataFrame to verify the headers
#print(df1.head())
#print(df2.head())
#print(df3.head())
#print(df4.head())


# Combine the DataFrames into a new column for each file, with missing rows left empty
df_combined = pd.merge(df1, df2, on='Opcode', how='outer', suffixes=(suffix1, suffix2))
df_combined = pd.merge(df_combined, df3, on='Opcode', how='outer', suffixes=('', suffix3))
df_combined = pd.merge(df_combined, df4, on='Opcode', how='outer', suffixes=('', suffix4))

# Save the combined DataFrame to a new CSV file
df_combined.to_csv('combined_file.csv', index=False)

print("The CSV files have been successfully combined and saved to 'combined_file.csv'.")

