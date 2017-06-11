Param (
 [String]$Project,
 [String]$GitRoot,
 [String]$ProjectURL="https://github.com/martb/retc/commit/"
)

Push-Location -LiteralPath $GitRoot

$FileHead = "`#pragma once`n`n"
$FileTail = "`n"

$Author   = (git log -n 1 --format=format:"#define VERSION_AUTHOR `\`"%an `<%ae`>`\`";%n") | Out-String
$Url  = (git log -n 1 --format=format:"#define VERSION_URL `\`"$ProjectURL%H`\`";%n") | Out-String
$Date = (git log -n 1 --format=format:"#define VERSION_DATE `\`"%ai`\`";%n") | Out-String
$CommitMsg = (git log -n 1 --format=format:"#define VERSION_CMSG `\`"%f`\`";%n") | Out-String
$GitTagLong = (git describe --long --tags)
$Tag  = ("#define VERSION_TAG `"$GitTagLong`";`n")

$VerNo = $GitTagLong -match '(?<=v).*?(?=-)'
$VerNo = $matches[0]
$VerNoAr = $VerNo.Split(".")
$VerNo = ""
for ($i=0;$i -lt 4; $i++) {
	$v = "0"
	if ($i -lt $VerNoAr.length) {
		$v = $VerNoAr[$i]
	}
	
	$VerNo = $VerNo + $(if ($i -ne 0) {","}) + $v
}

$VerNo = "#define VERSION_NUMBER "+$VerNo+";`n"

"Version updated for $Project\" + (
  New-Item -Force -Path "$Project" -Name "version.h" -ItemType "file" -Value "$FileHead$Url$Author$Date$Tag$VerNo$CommitMsg$FileTail"
).Name + " Contents:"
""
Get-Content "$Project\version.h"
""

Pop-Location