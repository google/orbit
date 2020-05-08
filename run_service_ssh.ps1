# This script first runs the ggp ssh init command to get the needed 
# credentials to connect via ssh. Then it connects to the instance 
# with portforwarding and starts OrbitService. To also deploy OrbitService,
# invoke with a the argument -deploy and a path to the OrbitService executable
# e.g. run_service_ssh.ps1 -deploy build/bin/OrbitService
# All other arguments will be passed to OrbitService.

param (
    [Parameter(Mandatory=$false)][string]$deploy,
    [Parameter(ValueFromRemainingArguments = $true)]$remainingArgs
 )

$ggp_sdk_path = $env:GGP_SDK_PATH

IF(!$ggp_sdk_path) {
    throw "GGP_SDK_PATH environment variable not found"
}

$ggp_path = "$($ggp_sdk_path)dev\bin\ggp.exe"

IF($deploy) {
    $ggp_put_command = "`"$($ggp_path)`" ssh put `"$($deploy)`""
    Invoke-Expression "& $ggp_put_command"
    $ggp_chmod_command = "`"$($ggp_path)`" ssh shell -- chmod u+x /mnt/developer/OrbitService"
    Invoke-Expression "& $ggp_chmod_command"
}

$ggp_ssh_init_command = "`"$($ggp_path)`" ssh init"

Invoke-Expression "& $ggp_ssh_init_command" | Tee-Object -Variable ggp_out

$lines = $ggp_out -split "\n"

ForEach ($line in $lines) {
    if ($line.Contains("User:")) {
        $ggp_user = $line.Replace("User:", "").Trim()
    }
    if ($line.Contains("Host:")) {
        $ggp_host = $line.Replace("Host:", "").Trim()
    }
    if ($line.Contains("Port:")) {
        $ggp_port = $line.Replace("Port:", "").Trim()
    }
    if ($line.Contains("Key Path:")) {
        $ggp_key_path = $line.Replace("Key Path:", "").Trim()
    }
    if ($line.Contains("Known Hosts Path:")) {
        $ggp_known_hosts_path = $line.Replace("Known Hosts Path:", "").Trim()
    }
}

IF (!$ggp_user -Or !$ggp_host -Or !$ggp_port -Or !$ggp_key_path -Or !$ggp_known_hosts_path) {
  throw "Unable to get all necessary information from ggp ssh init"
}

$ssh_path = "$($ggp_sdk_path)tools\OpenSSH-Win64\ssh.exe"

$ssh_command = "`"$($ssh_path)`" -t -p`"$($ggp_port)`" -i`"$($ggp_key_path)`" -oStrictHostKeyChecking=yes -oUserKnownHostsFile=`"$($ggp_known_hosts_path)`" -L44766:localhost:44766 -L44755:localhost:44755 $($ggp_user)@$($ggp_host) -- sudo /mnt/developer/OrbitService $($remainingArgs)"

Invoke-Expression "& $ssh_command"

