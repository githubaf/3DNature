/* SelectBuild.rexx
*/

parse arg buildname junk
options results

call open file, ('BuildIDs'), "R";stop = 0

do until stop = 1

  foo = readln(file)
  if upper(left(foo,length(buildname))) = upper(buildname) then
  do
    stop = 1
    args = words(foo)
  
    BuildID = word(foo, args)
    say "Configuring Beta release for "buildname". BuildID: 0x"BuildID
    address command
    "setenv BuildID 0x"BuildID
    address
  end
  if upper(left(foo,length("TEMP"))) = upper("TEMP") then
  do
    stop = 1
    args = words(foo)
  
    BuildID = word(foo, args)
    say "Configuring" BuildID "Beta release..."

    address command
    "setenv BuildID "BuildID
    address
  end
end

call close file
