if iteration equals to 1
    https://rpaexercise.aisingapore.org/login
    type //*[@name="username"] as jane007
    type //*[@name="password"] as TheBestHR123[enter]
https://rpaexercise.aisingapore.org/jobs
click New Job Posting

type Job Title as [clear]`jobTitle`
type Job Description as [clear]`jobDescription`
//clipboard(jobDescription)
//keyboard [ctrl]v

select  hiringDepartment as `hiringDepartment`
select  educationLevel as `educationLevel`

//click //*[@name="educationLevel"]
//type q as `educationLevel`
//click `educationLevel`

type Posting Start Date as [clear]`postingStartDate`
type Posting End Date as [clear]`postingEndDate`
click //*[@value="`remote`"]

if jobType contains "Full-time"
    click Full-time
if jobType contains "Permanent"
    click Permanent
if jobType contains "Part-time"
    click Part-time
if jobType contains "Temp"
    click Temp

click Submit
