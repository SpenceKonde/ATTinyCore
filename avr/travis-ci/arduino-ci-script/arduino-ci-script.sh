#!/bin/bash
# This script is used to automate continuous integration tasks for Arduino projects
# https://github.com/per1234/arduino-ci-script

# Based on https://github.com/adafruit/travis-ci-arduino/blob/eeaeaf8fa253465d18785c2bb589e14ea9893f9f/install.sh#L11
# It seems that arrays can't been seen in other functions. So instead I'm setting $IDE_VERSIONS to a string that is the command to create the array
readonly ARDUINO_CI_SCRIPT_IDE_VERSION_LIST_ARRAY_DECLARATION="declare -a -r IDEversionListArray="

readonly ARDUINO_CI_SCRIPT_TEMPORARY_FOLDER="${HOME}/temporary/arduino-ci-script"
readonly ARDUINO_CI_SCRIPT_IDE_INSTALLATION_FOLDER="arduino"
readonly ARDUINO_CI_SCRIPT_VERIFICATION_OUTPUT_FILENAME="${ARDUINO_CI_SCRIPT_TEMPORARY_FOLDER}/verification_output.txt"
readonly ARDUINO_CI_SCRIPT_REPORT_FILENAME="travis_ci_job_report_$(printf '%05d\n' "${TRAVIS_BUILD_NUMBER}").$(printf '%03d\n' "$(echo "$TRAVIS_JOB_NUMBER" | cut -d'.' -f 2)").tsv"
readonly ARDUINO_CI_SCRIPT_REPORT_FOLDER="${HOME}/arduino-ci-script_report"
readonly ARDUINO_CI_SCRIPT_REPORT_FILE_PATH="${ARDUINO_CI_SCRIPT_REPORT_FOLDER}/${ARDUINO_CI_SCRIPT_REPORT_FILENAME}"
# The arduino manpage(https://github.com/arduino/Arduino/blob/master/build/shared/manpage.adoc#exit-status) documents a range of exit statuses. These exit statuses indicate success, invalid arduino command, or compilation failed due to legitimate code errors. arduino sometimes returns other exit statuses that may indicate problems that may go away after a retry.
readonly ARDUINO_CI_SCRIPT_HIGHEST_ACCEPTABLE_ARDUINO_EXIT_STATUS=4
readonly ARDUINO_CI_SCRIPT_SKETCH_VERIFY_RETRIES=3
readonly ARDUINO_CI_SCRIPT_REPORT_PUSH_RETRIES=10

readonly ARDUINO_CI_SCRIPT_SUCCESS_EXIT_STATUS=0
readonly ARDUINO_CI_SCRIPT_FAILURE_EXIT_STATUS=1

# Arduino IDE 1.8.2 and newer generates a ton of garbage output (appears to be something related to jmdns) that must be filtered for the log to be readable and to avoid exceeding the maximum log length
readonly ARDUINO_CI_SCRIPT_ARDUINO_OUTPUT_FILTER_REGEX='(^\[SocketListener\(travis-job-*|^  *[0-9][0-9]*: [0-9a-g][0-9a-g]*|^dns\[query,[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*:[0-9][0-9]*, length=[0-9][0-9]*, id=|^dns\[response,[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*:[0-9][0-9]*, length=[0-9][0-9]*, id=|^questions:$|\[DNSQuestion@|type: TYPE_IGNORE|^\.\]$|^\.\]\]$|^.\.\]$|^.\.\]\]$)'

# Default value
ARDUINO_CI_SCRIPT_TOTAL_SKETCH_BUILD_FAILURE_COUNT=0

# Set the arduino command name according to OS (on Windows arduino_debug should be used)
if [[ "$OSTYPE" == "cygwin" ]] || [[ "$OSTYPE" == "msys" ]]; then
  ARDUINO_CI_SCRIPT_ARDUINO_COMMAND="arduino_debug"
else
  ARDUINO_CI_SCRIPT_ARDUINO_COMMAND="arduino"
fi

# Create the folder if it doesn't exist
function create_folder() {
  local -r folderName="$1"
  if ! [[ -d "$folderName" ]]; then
    # shellcheck disable=SC2086
    mkdir --parents $ARDUINO_CI_SCRIPT_VERBOSITY_OPTION "$folderName"
  fi
}

function set_script_verbosity() {
  ARDUINO_CI_SCRIPT_VERBOSITY_LEVEL="$1"

  if [[ "$ARDUINO_CI_SCRIPT_VERBOSITY_LEVEL" == "true" ]]; then
    ARDUINO_CI_SCRIPT_VERBOSITY_LEVEL=1
  fi

  if [[ "$ARDUINO_CI_SCRIPT_VERBOSITY_LEVEL" -eq 1 ]]; then
    ARDUINO_CI_SCRIPT_VERBOSITY_OPTION="--verbose"
    ARDUINO_CI_SCRIPT_QUIET_OPTION=""
    # Show stderr only
    ARDUINO_CI_SCRIPT_VERBOSITY_REDIRECT="1>/dev/null"
  elif [[ "$ARDUINO_CI_SCRIPT_VERBOSITY_LEVEL" -eq 2 ]]; then
    ARDUINO_CI_SCRIPT_VERBOSITY_OPTION="--verbose"
    ARDUINO_CI_SCRIPT_QUIET_OPTION=""
    # Show stdout and stderr
    ARDUINO_CI_SCRIPT_VERBOSITY_REDIRECT=""
  else
    ARDUINO_CI_SCRIPT_VERBOSITY_LEVEL=0
    ARDUINO_CI_SCRIPT_VERBOSITY_OPTION=""
    # cabextract only takes the short option name so this is more universally useful than --quiet
    ARDUINO_CI_SCRIPT_QUIET_OPTION="-q"
    # Don't show stderr or stdout
    ARDUINO_CI_SCRIPT_VERBOSITY_REDIRECT="&>/dev/null"
  fi

}

# Deprecated, use set_script_verbosity
function set_verbose_script_output() {
  set_script_verbosity 1
}

# Deprecated, use set_script_verbosity
function set_more_verbose_script_output() {
  set_script_verbosity 2
}

# Turn on verbosity based on the preferences set by set_script_verbosity
function enable_verbosity() {
  # Store previous verbosity settings so they can be set back to their original values at the end of the function
  shopt -q -o verbose
  ARDUINO_CI_SCRIPT_PREVIOUS_VERBOSE_SETTING="$?"

  shopt -q -o xtrace
  ARDUINO_CI_SCRIPT_PREVIOUS_XTRACE_SETTING="$?"

  if [[ "$ARDUINO_CI_SCRIPT_VERBOSITY_LEVEL" -gt 0 ]]; then
    # "Print shell input lines as they are read."
    # https://www.gnu.org/software/bash/manual/html_node/The-Set-Builtin.html
    set -o verbose
  fi
  if [[ "$ARDUINO_CI_SCRIPT_VERBOSITY_LEVEL" -gt 1 ]]; then
    # "Print a trace of simple commands, for commands, case commands, select commands, and arithmetic for commands and their arguments or associated word lists after they are expanded and before they are executed. The value of the PS4 variable is expanded and the resultant value is printed before the command and its expanded arguments."
    # https://www.gnu.org/software/bash/manual/html_node/The-Set-Builtin.html
    set -o xtrace
  fi
}

# Return verbosity settings to their previous values
function disable_verbosity() {
  if [[ "$ARDUINO_CI_SCRIPT_PREVIOUS_VERBOSE_SETTING" == "0" ]]; then
    set -o verbose
  else
    set +o verbose
  fi

  if [[ "$ARDUINO_CI_SCRIPT_PREVIOUS_XTRACE_SETTING" == "0" ]]; then
    set -o xtrace
  else
    set +o xtrace
  fi
}

# Verbosity and, in some cases, errexit must be disabled before an early return from a public function, this allows it to be done in a single line instead of two
function return_handler() {
  local -r exitStatus="$1"

  # If exit status is success and errexit is enabled then it must be disabled before exiting the script because errexit must be disabled by default and only enabled in the functions that specifically require it.
  # If exit status is not success then errexit should not be disabled, otherwise Travis CI won't fail the build even though the exit status was failure.
  if [[ "$exitStatus" == "$ARDUINO_CI_SCRIPT_SUCCESS_EXIT_STATUS" ]] && shopt -q -o errexit; then
    set +o errexit
  fi

  disable_verbosity

  return "$exitStatus"
}

function set_application_folder() {
  enable_verbosity

  ARDUINO_CI_SCRIPT_APPLICATION_FOLDER="$1"

  disable_verbosity
}

function set_sketchbook_folder() {
  enable_verbosity

  ARDUINO_CI_SCRIPT_SKETCHBOOK_FOLDER="$1"

  # Create the sketchbook folder if it doesn't already exist
  create_folder "$ARDUINO_CI_SCRIPT_SKETCHBOOK_FOLDER"

  # Set sketchbook location preference if the IDE is already installed
  if [[ "$INSTALLED_IDE_VERSION_LIST_ARRAY" != "" ]]; then
    set_ide_preference "sketchbook.path=$ARDUINO_CI_SCRIPT_SKETCHBOOK_FOLDER"
  fi

  disable_verbosity
}

# Deprecated
function set_parameters() {
  set_application_folder "$1"
  set_sketchbook_folder "$2"
}

# Check for errors with the board definition that don't affect sketch verification
function set_board_testing() {
  enable_verbosity

  ARDUINO_CI_SCRIPT_TEST_BOARD="$1"

  disable_verbosity
}

# Check for errors with libraries that don't affect sketch verification
function set_library_testing() {
  enable_verbosity

  ARDUINO_CI_SCRIPT_TEST_LIBRARY="$1"

  disable_verbosity
}

# Install all specified versions of the Arduino IDE
function install_ide() {
  enable_verbosity

  local -r startIDEversion="$1"
  local -r endIDEversion="$2"

  # https://docs.travis-ci.com/user/customizing-the-build/#Implementing-Complex-Build-Steps
  # set -o errexit will cause the script to exit as soon as any command returns a non-zero exit status. Without this the success of the function call is determined by the exit status of the last command in the function
  set -o errexit

  if [[ "$ARDUINO_CI_SCRIPT_APPLICATION_FOLDER" == "" ]]; then
    echo "ERROR: Application folder was not set. Please use the set_application_folder function to define the location of the application folder."
    return_handler "$ARDUINO_CI_SCRIPT_FAILURE_EXIT_STATUS"
  fi

  # Generate an array declaration string containing a list all Arduino IDE versions which support CLI (1.5.2+ according to https://github.com/arduino/Arduino/blob/master/build/shared/manpage.adoc#history)
  # Save the current folder
  local -r previousFolder="$PWD"
  cd "$ARDUINO_CI_SCRIPT_TEMPORARY_FOLDER"
  # Create empty local repo for the purpose of getting a list of tags in the arduino/Arduino repository
  git init --quiet Arduino
  cd Arduino
  git remote add origin https://github.com/arduino/Arduino.git
  if [[ "$startIDEversion" != "1.6.2" ]] && [[ "$startIDEversion" != "1.6.2" ]]; then
    # Arduino IDE 1.6.2 has the nasty behavior of moving the included hardware cores to the .arduino15 folder, causing those versions to be used for all builds after Arduino IDE 1.6.2 is used. For that reason, 1.6.2 will only be installed if explicitly specified in the install_ide version arguments
    local -r IDEversion162regex=--regex='refs/tags/1\.6\.2'
    if [[ "$ARDUINO_CI_SCRIPT_VERBOSITY_LEVEL" -gt 0 ]]; then
      echo "NOTE: Due to not playing nicely with other versions, Arduino IDE 1.6.2 will not be installed unless explicitly specified in the version arguments."
    fi
  fi
  local -r ARDUINO_CI_SCRIPT_FULL_IDE_VERSION_LIST_ARRAY="${ARDUINO_CI_SCRIPT_IDE_VERSION_LIST_ARRAY_DECLARATION}'(\"$(git ls-remote --quiet --tags --refs | grep --invert-match --regexp='refs/tags/1\.0' --regexp='refs/tags/1\.5$' --regexp='refs/tags/1\.5\.1$' --regexp='refs/tags/1\.5\.4-r2$' --regexp='refs/tags/1\.5\.5-r2$' --regexp='refs/tags/1\.5\.7-macosx-java7$' --regexp='refs/tags/1\.5\.8-macosx-java7$' ${IDEversion162regex} --regexp='refs/tags/1\.6\.5-r2$' --regexp='refs/tags/1\.6\.5-r3$' | grep --regexp='refs/tags/[0-9]\+\.[0-9]\+\.[0-9]\+\(\(-.*$\)\|$\)' | cut --delimiter='/' --fields=3 | sort --version-sort | sed ':a;N;$!ba;s/\n/\" \"/g')\")'"
  cd ..
  # Remove the temporary repo
  rm Arduino --recursive --force
  # Go back to the previous folder location
  cd "$previousFolder"

  # Determine list of IDE versions to install
  generate_ide_version_list_array "$ARDUINO_CI_SCRIPT_FULL_IDE_VERSION_LIST_ARRAY" "$startIDEversion" "$endIDEversion"
  INSTALLED_IDE_VERSION_LIST_ARRAY="$ARDUINO_CI_SCRIPT_GENERATED_IDE_VERSION_LIST_ARRAY"

  # Set "$NEWEST_INSTALLED_IDE_VERSION"
  determine_ide_version_extremes "$INSTALLED_IDE_VERSION_LIST_ARRAY"
  NEWEST_INSTALLED_IDE_VERSION="$ARDUINO_CI_SCRIPT_DETERMINED_NEWEST_IDE_VERSION"

  create_folder "$ARDUINO_CI_SCRIPT_APPLICATION_FOLDER"

  # This runs the command contained in the $INSTALLED_IDE_VERSION_LIST_ARRAY string, thus declaring the array locally as $IDEversionListArray. This must be done in any function that uses the array
  # Dummy declaration to fix the "referenced but not assigned" warning.
  local IDEversionListArray
  eval "$INSTALLED_IDE_VERSION_LIST_ARRAY"

  # Determine whether any of the IDE versions to be installed require the creation of a virtual framebuffer (https://github.com/arduino/Arduino/blob/54264124b72eec40aaa22e327c16760f5e806c2a/build/shared/manpage.adoc#bugs)
  # This is necessary in Arduino IDE 1.6.13 and older (https://github.com/arduino/Arduino/pull/5578) when running on a headless system
  if [ -e /usr/bin/Xvfb ]; then
    local -r virtualFramebufferRequiredRegex='^1\.[56]\.'
    local IDEversion
    for IDEversion in "${IDEversionListArray[@]}"; do
      if [[ "$IDEversion" =~ $virtualFramebufferRequiredRegex ]]; then
        # based on https://learn.adafruit.com/continuous-integration-arduino-and-you/testing-your-project
        /sbin/start-stop-daemon --start $ARDUINO_CI_SCRIPT_QUIET_OPTION $ARDUINO_CI_SCRIPT_VERBOSITY_OPTION --pidfile /tmp/custom_xvfb_1.pid --make-pidfile --background --exec /usr/bin/Xvfb -- :1 -ac -screen 0 1280x1024x16
        sleep 3
        export DISPLAY=:1.0
        break
      fi
    done
  fi

  local IDEversion
  for IDEversion in "${IDEversionListArray[@]}"; do
    local IDEinstallFolder="$ARDUINO_CI_SCRIPT_APPLICATION_FOLDER/arduino-${IDEversion}"

    # Don't unnecessarily install the IDE
    if ! [[ -d "$IDEinstallFolder" ]]; then
      if [[ "$ARDUINO_CI_SCRIPT_VERBOSITY_LEVEL" -eq 0 ]]; then
        # If the download/installation process is going slowly when installing a lot of IDE versions this function may cause the build to fail due to exceeding Travis CI's 10 minutes without log output timeout so it's necessary to periodically print something.
        echo "Installing: $IDEversion"
      fi

      # Determine download file extension
      local tgzExtensionVersionsRegex="^1\.5\.[0-9]$"
      if [[ "$IDEversion" =~ $tgzExtensionVersionsRegex ]]; then
        # The download file extension prior to 1.6.0 is .tgz
        local downloadFileExtension="tgz"
      else
        local downloadFileExtension="tar.xz"
      fi

      if [[ "$IDEversion" == "hourly" ]]; then
        # Deal with the inaccurate name given to the hourly build download
        local downloadVersion="nightly"
      else
        local downloadVersion="$IDEversion"
      fi

      wget --no-verbose $ARDUINO_CI_SCRIPT_QUIET_OPTION --directory-prefix="${ARDUINO_CI_SCRIPT_TEMPORARY_FOLDER}/" "http://downloads.arduino.cc/arduino-${downloadVersion}-linux64.${downloadFileExtension}"
      tar --extract --directory="$ARDUINO_CI_SCRIPT_TEMPORARY_FOLDER" --file="${ARDUINO_CI_SCRIPT_TEMPORARY_FOLDER}/arduino-${downloadVersion}-linux64.${downloadFileExtension}"
      rm $ARDUINO_CI_SCRIPT_VERBOSITY_OPTION "${ARDUINO_CI_SCRIPT_TEMPORARY_FOLDER}/arduino-${downloadVersion}-linux64.${downloadFileExtension}"
      mv $ARDUINO_CI_SCRIPT_VERBOSITY_OPTION "${ARDUINO_CI_SCRIPT_TEMPORARY_FOLDER}/arduino-${downloadVersion}" "$IDEinstallFolder"
    fi
  done

  set_ide_preference "compiler.warning_level=all"

  # If a sketchbook location has been defined then set the location in the Arduino IDE preferences
  if [[ -d "$ARDUINO_CI_SCRIPT_SKETCHBOOK_FOLDER" ]]; then
    set_ide_preference "sketchbook.path=$ARDUINO_CI_SCRIPT_SKETCHBOOK_FOLDER"
  fi

  # Return errexit to the default state
  set +o errexit

  disable_verbosity
}

# Generate an array of Arduino IDE versions as a subset of the list provided in the base array defined by the start and end versions
# This function allows the same code to be shared by install_ide and build_sketch. The generated array is "returned" as a global named "$ARDUINO_CI_SCRIPT_GENERATED_IDE_VERSION_LIST_ARRAY"
function generate_ide_version_list_array() {
  local -r baseIDEversionArray="$1"
  local startIDEversion="$2"
  local endIDEversion="$3"

  # Convert "oldest" or "newest" to actual version numbers
  determine_ide_version_extremes "$baseIDEversionArray"
  if [[ "$startIDEversion" == "oldest" ]]; then
    startIDEversion="$ARDUINO_CI_SCRIPT_DETERMINED_OLDEST_IDE_VERSION"
  elif [[ "$startIDEversion" == "newest" ]]; then
    startIDEversion="$ARDUINO_CI_SCRIPT_DETERMINED_NEWEST_IDE_VERSION"
  fi

  if [[ "$endIDEversion" == "oldest" ]]; then
    endIDEversion="$ARDUINO_CI_SCRIPT_DETERMINED_OLDEST_IDE_VERSION"
  elif [[ "$endIDEversion" == "newest" ]]; then
    endIDEversion="$ARDUINO_CI_SCRIPT_DETERMINED_NEWEST_IDE_VERSION"
  fi

  if [[ "$startIDEversion" == "" || "$startIDEversion" == "all" ]]; then
    # Use the full base array
    ARDUINO_CI_SCRIPT_GENERATED_IDE_VERSION_LIST_ARRAY="$baseIDEversionArray"

  else
    local rawIDElist
    local -r IDEversionListRegex='\('
    if [[ "$startIDEversion" =~ $IDEversionListRegex ]]; then
      # IDE versions list was supplied
      # Convert it to a temporary array
      local -r suppliedIDEversionListArray="${ARDUINO_CI_SCRIPT_IDE_VERSION_LIST_ARRAY_DECLARATION}${startIDEversion}"
      eval "$suppliedIDEversionListArray"
      local IDEversion
      for IDEversion in "${IDEversionListArray[@]}"; do
        # Convert any use of "oldest" or "newest" special version names to the actual version number
        if [[ "$IDEversion" == "oldest" ]]; then
          IDEversion="$ARDUINO_CI_SCRIPT_DETERMINED_OLDEST_IDE_VERSION"
        elif [[ "$IDEversion" == "newest" ]]; then
          IDEversion="$ARDUINO_CI_SCRIPT_DETERMINED_NEWEST_IDE_VERSION"
        fi
        # Add the version to the array
        rawIDElist="${rawIDElist} "'"'"$IDEversion"'"'
      done

    elif [[ "$endIDEversion" == "" ]]; then
      # Only a single version was specified
      rawIDElist="$rawIDElist"'"'"$startIDEversion"'"'

    else
      # A version range was specified
      eval "$baseIDEversionArray"
      local IDEversion
      for IDEversion in "${IDEversionListArray[@]}"; do
        if [[ "$IDEversion" == "$startIDEversion" ]]; then
          # Start of the list reached, set a flag
          local -r listIsStarted="true"
        fi

        if [[ "$listIsStarted" == "true" ]]; then
          # Add the version to the list
          rawIDElist="${rawIDElist} "'"'"$IDEversion"'"'
        fi

        if [[ "$IDEversion" == "$endIDEversion" ]]; then
          # End of the list was reached, exit the loop
          break
        fi
      done
    fi

    # Turn the raw IDE version list into an array
    declare -a -r rawIDElistArray="(${rawIDElist})"

    # Remove duplicates from list https://stackoverflow.com/a/13648438
    # shellcheck disable=SC2207
    readonly local uniqueIDElistArray=($(echo "${rawIDElistArray[@]}" | tr ' ' '\n' | sort --unique --version-sort | tr '\n' ' '))

    # Generate ARDUINO_CI_SCRIPT_GENERATED_IDE_VERSION_LIST_ARRAY
    ARDUINO_CI_SCRIPT_GENERATED_IDE_VERSION_LIST_ARRAY="$ARDUINO_CI_SCRIPT_IDE_VERSION_LIST_ARRAY_DECLARATION"'('
    for uniqueIDElistArrayIndex in "${!uniqueIDElistArray[@]}"; do
      ARDUINO_CI_SCRIPT_GENERATED_IDE_VERSION_LIST_ARRAY="${ARDUINO_CI_SCRIPT_GENERATED_IDE_VERSION_LIST_ARRAY} "'"'"${uniqueIDElistArray[$uniqueIDElistArrayIndex]}"'"'
    done
    ARDUINO_CI_SCRIPT_GENERATED_IDE_VERSION_LIST_ARRAY="$ARDUINO_CI_SCRIPT_GENERATED_IDE_VERSION_LIST_ARRAY"')'
  fi
}

# Determine the oldest and newest (non-hourly unless hourly is the only version on the list) IDE version in the provided array
# The determined versions are "returned" by setting the global variables "$ARDUINO_CI_SCRIPT_DETERMINED_OLDEST_IDE_VERSION" and "$ARDUINO_CI_SCRIPT_DETERMINED_NEWEST_IDE_VERSION"
function determine_ide_version_extremes() {
  local -r baseIDEversionArray="$1"

  # Reset the variables from any value they were assigned the last time the function was ran
  ARDUINO_CI_SCRIPT_DETERMINED_OLDEST_IDE_VERSION=""
  ARDUINO_CI_SCRIPT_DETERMINED_NEWEST_IDE_VERSION=""

  # Determine the oldest and newest (non-hourly) IDE version in the base array
  eval "$baseIDEversionArray"
  local IDEversion
  for IDEversion in "${IDEversionListArray[@]}"; do
    if [[ "$ARDUINO_CI_SCRIPT_DETERMINED_OLDEST_IDE_VERSION" == "" ]]; then
      ARDUINO_CI_SCRIPT_DETERMINED_OLDEST_IDE_VERSION="$IDEversion"
    fi
    if [[ "$ARDUINO_CI_SCRIPT_DETERMINED_NEWEST_IDE_VERSION" == "" || "$IDEversion" != "hourly" ]]; then
      ARDUINO_CI_SCRIPT_DETERMINED_NEWEST_IDE_VERSION="$IDEversion"
    fi
  done
}

function set_ide_preference() {
  local -r preferenceString="$1"

  # --pref option is only supported by Arduino IDE 1.5.6 and newer
  local -r unsupportedPrefOptionVersionsRegex="^1\.5\.[0-5]$"
  if ! [[ "$NEWEST_INSTALLED_IDE_VERSION" =~ $unsupportedPrefOptionVersionsRegex ]]; then
    install_ide_version "$NEWEST_INSTALLED_IDE_VERSION"

    # --save-prefs was added in Arduino IDE 1.5.8
    local -r unsupportedSavePrefsOptionVersionsRegex="^1\.5\.[6-7]$"
    if ! [[ "$NEWEST_INSTALLED_IDE_VERSION" =~ $unsupportedSavePrefsOptionVersionsRegex ]]; then
      # shellcheck disable=SC2086
      eval \"${ARDUINO_CI_SCRIPT_APPLICATION_FOLDER}/${ARDUINO_CI_SCRIPT_IDE_INSTALLATION_FOLDER}/${ARDUINO_CI_SCRIPT_ARDUINO_COMMAND}\" --pref "$preferenceString" --save-prefs "$ARDUINO_CI_SCRIPT_VERBOSITY_REDIRECT"
    else
      # Arduino IDE 1.5.6 - 1.5.7 load the GUI if you only set preferences without doing a verify. So I am doing an unnecessary verification just to set the preferences in those versions. Definitely a hack but I prefer to keep the preferences setting code all here instead of cluttering build_sketch and this will pretty much never be used.
      # shellcheck disable=SC2086
      eval \"${ARDUINO_CI_SCRIPT_APPLICATION_FOLDER}/${ARDUINO_CI_SCRIPT_IDE_INSTALLATION_FOLDER}/${ARDUINO_CI_SCRIPT_ARDUINO_COMMAND}\" --pref "$preferenceString" --verify "${ARDUINO_CI_SCRIPT_APPLICATION_FOLDER}/arduino/examples/01.Basics/BareMinimum/BareMinimum.ino" "$ARDUINO_CI_SCRIPT_VERBOSITY_REDIRECT"
    fi
  fi
}

function install_ide_version() {
  local -r IDEversion="$1"

  # Create a symbolic link so that the Arduino IDE can always be referenced by the user from the same path no matter which version is being used.
  if [[ "$OSTYPE" == "cygwin" ]] || [[ "$OSTYPE" == "msys" ]]; then
    # git-bash's ln just does a copy instead of making a symlink, which takes forever and fails when the target folder exists (despite --force), which takes forever.
    # Therefore, use the native Windows command mklink to create a directory junction instead.
    # Using a directory junction instead of symlink because supposedly a symlink requires admin privileges.

    # Windows doesn't seem to provide any way to overwrite directory junctions
    if [[ -d "${ARDUINO_CI_SCRIPT_APPLICATION_FOLDER}/${ARDUINO_CI_SCRIPT_IDE_INSTALLATION_FOLDER}" ]]; then
      rm --recursive --force "${ARDUINO_CI_SCRIPT_APPLICATION_FOLDER}/${ARDUINO_CI_SCRIPT_IDE_INSTALLATION_FOLDER:?}"
    fi
    # https://stackoverflow.com/a/25394801
    cmd <<<"mklink /J \"${ARDUINO_CI_SCRIPT_APPLICATION_FOLDER}\\${ARDUINO_CI_SCRIPT_IDE_INSTALLATION_FOLDER}\" \"${ARDUINO_CI_SCRIPT_APPLICATION_FOLDER//\//\\}\\arduino-${IDEversion}\"" >/dev/null
  else
    ln --symbolic --force --no-dereference $ARDUINO_CI_SCRIPT_VERBOSITY_OPTION "${ARDUINO_CI_SCRIPT_APPLICATION_FOLDER}/arduino-${IDEversion}" "${ARDUINO_CI_SCRIPT_APPLICATION_FOLDER}/${ARDUINO_CI_SCRIPT_IDE_INSTALLATION_FOLDER}"
  fi
}

# Install hardware packages
function install_package() {
  enable_verbosity

  set -o errexit

  local -r URLregex="://"
  if [[ "$1" =~ $URLregex ]]; then
    # First argument is a URL, do a manual hardware package installation
    # Note: Assumes the package is in the root of the download and has the correct folder structure (e.g. architecture folder added in Arduino IDE 1.5+)

    local -r packageURL="$1"

    # Create the hardware folder if it doesn't exist
    create_folder "${ARDUINO_CI_SCRIPT_SKETCHBOOK_FOLDER}/hardware"

    if [[ "$packageURL" =~ \.git$ ]]; then
      # Clone the repository
      local -r branchName="$2"

      local -r previousFolder="$PWD"
      cd "${ARDUINO_CI_SCRIPT_SKETCHBOOK_FOLDER}/hardware"

      if [[ "$branchName" == "" ]]; then
        git clone --quiet "$packageURL"
      else
        git clone --quiet --branch "$branchName" "$packageURL"
      fi
      cd "$previousFolder"
    else
      local -r previousFolder="$PWD"
      cd "$ARDUINO_CI_SCRIPT_TEMPORARY_FOLDER"

      # Delete everything from the temporary folder
      find ./ -mindepth 1 -delete

      # Download the package
      wget --no-verbose $ARDUINO_CI_SCRIPT_QUIET_OPTION "$packageURL"

      # Uncompress the package
      extract ./*.*

      # Delete all files from the temporary folder
      find ./ -maxdepth 1 -type f -delete

      # Install the package
      mv $ARDUINO_CI_SCRIPT_VERBOSITY_OPTION ./* "${ARDUINO_CI_SCRIPT_SKETCHBOOK_FOLDER}/hardware/"
      cd "$previousFolder"
    fi

  elif [[ "$1" == "" ]]; then
    # Install hardware package from this repository
    # https://docs.travis-ci.com/user/environment-variables#Global-Variables
    local packageName
    packageName="$(echo "$TRAVIS_REPO_SLUG" | cut -d'/' -f 2)"
    mkdir --parents $ARDUINO_CI_SCRIPT_VERBOSITY_OPTION "${ARDUINO_CI_SCRIPT_SKETCHBOOK_FOLDER}/hardware/$packageName"
    local -r previousFolder="$PWD"
    cd "$TRAVIS_BUILD_DIR"
    cp --recursive $ARDUINO_CI_SCRIPT_VERBOSITY_OPTION ./* "${ARDUINO_CI_SCRIPT_SKETCHBOOK_FOLDER}/hardware/${packageName}"
    # * doesn't copy .travis.yml but that file will be present in the user's installation so it should be there for the tests too
    cp $ARDUINO_CI_SCRIPT_VERBOSITY_OPTION "${TRAVIS_BUILD_DIR}/.travis.yml" "${ARDUINO_CI_SCRIPT_SKETCHBOOK_FOLDER}/hardware/${packageName}"
    cd "$previousFolder"

  else
    # Install package via Boards Manager

    local -r packageID="$1"
    local -r packageURL="$2"

    # Check if Arduino IDE is installed
    if [[ "$INSTALLED_IDE_VERSION_LIST_ARRAY" == "" ]]; then
      echo "ERROR: Installing a hardware package via Boards Manager requires the Arduino IDE to be installed. Please call install_ide before this command."
      return_handler "$ARDUINO_CI_SCRIPT_FAILURE_EXIT_STATUS"
    fi

    # Check if the newest installed IDE version supports --install-boards
    local -r unsupportedInstallBoardsOptionVersionsRange1regex="^1\.5\.[0-9]$"
    local -r unsupportedInstallBoardsOptionVersionsRange2regex="^1\.6\.[0-3]$"
    if [[ "$NEWEST_INSTALLED_IDE_VERSION" =~ $unsupportedInstallBoardsOptionVersionsRange1regex ]] || [[ "$NEWEST_INSTALLED_IDE_VERSION" =~ $unsupportedInstallBoardsOptionVersionsRange2regex ]]; then
      echo "ERROR: --install-boards option is not supported by the newest version of the Arduino IDE you have installed. You must have Arduino IDE 1.6.4 or newer installed to use this function."
      return_handler "$ARDUINO_CI_SCRIPT_FAILURE_EXIT_STATUS"
    else
      # Temporarily install the latest IDE version to use for the package installation
      install_ide_version "$NEWEST_INSTALLED_IDE_VERSION"

      # If defined add the boards manager URL to preferences
      if [[ "$packageURL" != "" ]]; then
        # Get the current Additional Boards Manager URLs preference value so it won't be overwritten when the new URL is added
        local priorBoardsmanagerAdditionalURLs
        if [[ "$ARDUINO_CI_SCRIPT_VERBOSITY_LEVEL" -eq 0 ]]; then
          priorBoardsmanagerAdditionalURLs=$("${ARDUINO_CI_SCRIPT_APPLICATION_FOLDER}/${ARDUINO_CI_SCRIPT_IDE_INSTALLATION_FOLDER}/${ARDUINO_CI_SCRIPT_ARDUINO_COMMAND}" --get-pref boardsmanager.additional.urls 2>/dev/null | tail --lines=1)
        elif [[ "$ARDUINO_CI_SCRIPT_VERBOSITY_LEVEL" -eq 1 ]]; then
          priorBoardsmanagerAdditionalURLs=$("${ARDUINO_CI_SCRIPT_APPLICATION_FOLDER}/${ARDUINO_CI_SCRIPT_IDE_INSTALLATION_FOLDER}/${ARDUINO_CI_SCRIPT_ARDUINO_COMMAND}" --get-pref boardsmanager.additional.urls | tail --lines=1)
        else
          priorBoardsmanagerAdditionalURLs=$("${ARDUINO_CI_SCRIPT_APPLICATION_FOLDER}/${ARDUINO_CI_SCRIPT_IDE_INSTALLATION_FOLDER}/${ARDUINO_CI_SCRIPT_ARDUINO_COMMAND}" --get-pref boardsmanager.additional.urls | tee /dev/tty | tail --lines=1)
        fi
        local -r blankregex="^[ ]*$"
        if [[ "$priorBoardsmanagerAdditionalURLs" =~ $blankregex ]]; then
          # There is no previous Additional Boards Manager URLs preference value
          local boardsmanagerAdditionalURLs="$packageURL"
        else
          # There is a previous Additional Boards Manager URLs preference value so append the new one to the end of it
          local boardsmanagerAdditionalURLs="${priorBoardsmanagerAdditionalURLs},${packageURL}"
        fi

        # grep returns 1 when a line matches the regular expression so it's necessary to unset errexit
        set +o errexit
        # shellcheck disable=SC2086
        eval \"${ARDUINO_CI_SCRIPT_APPLICATION_FOLDER}/${ARDUINO_CI_SCRIPT_IDE_INSTALLATION_FOLDER}/${ARDUINO_CI_SCRIPT_ARDUINO_COMMAND}\" --pref boardsmanager.additional.urls="$boardsmanagerAdditionalURLs" --save-prefs "$ARDUINO_CI_SCRIPT_VERBOSITY_REDIRECT" | tr --complement --delete '[:print:]\n\t' | tr --squeeze-repeats '\n' | grep --extended-regexp --invert-match "$ARDUINO_CI_SCRIPT_ARDUINO_OUTPUT_FILTER_REGEX"
        local -r arduinoPreferenceSettingExitStatus="${PIPESTATUS[0]}"
        set -o errexit
        # this is required because otherwise the exit status of arduino is ignored
        if [[ "$arduinoPreferenceSettingExitStatus" != "$ARDUINO_CI_SCRIPT_SUCCESS_EXIT_STATUS" ]]; then
          return_handler "$arduinoPreferenceSettingExitStatus"
        fi
      fi

      # Install the package
      # grep returns 1 when a line matches the regular expression so it's necessary to unset errexit
      set +o errexit
      # shellcheck disable=SC2086
      eval \"${ARDUINO_CI_SCRIPT_APPLICATION_FOLDER}/${ARDUINO_CI_SCRIPT_IDE_INSTALLATION_FOLDER}/${ARDUINO_CI_SCRIPT_ARDUINO_COMMAND}\" --install-boards "$packageID" "$ARDUINO_CI_SCRIPT_VERBOSITY_REDIRECT" | tr --complement --delete '[:print:]\n\t' | tr --squeeze-repeats '\n' | grep --extended-regexp --invert-match "$ARDUINO_CI_SCRIPT_ARDUINO_OUTPUT_FILTER_REGEX"
      local -r arduinoInstallPackageExitStatus="${PIPESTATUS[0]}"
      set -o errexit
      # this is required because otherwise the exit status of arduino is ignored
      if [[ "$arduinoInstallPackageExitStatus" != "$ARDUINO_CI_SCRIPT_SUCCESS_EXIT_STATUS" ]]; then
        return_handler "$arduinoPreferenceSettingExitStatus"
      fi

    fi
  fi

  set +o errexit

  disable_verbosity
}

function install_library() {
  enable_verbosity

  set -o errexit

  local -r libraryIdentifier="$1"

  # Create the libraries folder if it doesn't already exist
  create_folder "${ARDUINO_CI_SCRIPT_SKETCHBOOK_FOLDER}/libraries"

  local -r URLregex="://"
  if [[ "$libraryIdentifier" =~ $URLregex ]]; then
    # The argument is a URL
    # Note: this assumes the library is in the root of the file
    if [[ "$libraryIdentifier" =~ \.git$ ]]; then
      # Clone the repository
      local -r branchName="$2"
      local -r newFolderName="$3"

      local -r previousFolder="$PWD"
      cd "${ARDUINO_CI_SCRIPT_SKETCHBOOK_FOLDER}/libraries"

      if [[ "$branchName" == "" && "$newFolderName" == "" ]]; then
        git clone --quiet "$libraryIdentifier"
      elif [[ "$branchName" == "" ]]; then
        git clone --quiet "$libraryIdentifier" "$newFolderName"
      elif [[ "$newFolderName" == "" ]]; then
        git clone --quiet --branch "$branchName" "$libraryIdentifier"
      else
        git clone --quiet --branch "$branchName" "$libraryIdentifier" "$newFolderName"
      fi
      cd "$previousFolder"
    else
      # Assume it's a compressed file
      local -r newFolderName="$2"
      # Download the file to the temporary folder
      local -r previousFolder="$PWD"
      cd "$ARDUINO_CI_SCRIPT_TEMPORARY_FOLDER"

      # Delete everything from the temporary folder
      find ./ -mindepth 1 -delete

      wget --no-verbose $ARDUINO_CI_SCRIPT_QUIET_OPTION "$libraryIdentifier"

      extract ./*.*

      # Delete all files from the temporary folder
      find ./ -maxdepth 1 -type f -delete

      # Install the library
      mv $ARDUINO_CI_SCRIPT_VERBOSITY_OPTION ./* "${ARDUINO_CI_SCRIPT_SKETCHBOOK_FOLDER}/libraries/${newFolderName}"
      cd "$previousFolder"
    fi

  elif [[ "$libraryIdentifier" == "" ]]; then
    # Install library from the repository
    # https://docs.travis-ci.com/user/environment-variables#Global-Variables
    local libraryName
    libraryName="$(echo "$TRAVIS_REPO_SLUG" | cut -d'/' -f 2)"
    mkdir --parents $ARDUINO_CI_SCRIPT_VERBOSITY_OPTION "${ARDUINO_CI_SCRIPT_SKETCHBOOK_FOLDER}/libraries/$libraryName"
    local -r previousFolder="$PWD"
    cd "$TRAVIS_BUILD_DIR"
    cp --recursive $ARDUINO_CI_SCRIPT_VERBOSITY_OPTION ./* "${ARDUINO_CI_SCRIPT_SKETCHBOOK_FOLDER}/libraries/${libraryName}"
    # * doesn't copy .travis.yml but that file will be present in the user's installation so it should be there for the tests too
    cp $ARDUINO_CI_SCRIPT_VERBOSITY_OPTION "${TRAVIS_BUILD_DIR}/.travis.yml" "${ARDUINO_CI_SCRIPT_SKETCHBOOK_FOLDER}/libraries/${libraryName}"
    cd "$previousFolder"

  else
    # Install a library that is part of the Library Manager index

    # Check if Arduino IDE is installed
    if [[ "$INSTALLED_IDE_VERSION_LIST_ARRAY" == "" ]]; then
      echo "ERROR: Installing a library via Library Manager requires the Arduino IDE to be installed. Please call install_ide before this command."
      return_handler "$ARDUINO_CI_SCRIPT_FAILURE_EXIT_STATUS"
    fi

    # Check if the newest installed IDE version supports --install-library
    local -r unsupportedInstallLibraryOptionVersionsRange1regex="^1\.5\.[0-9]$"
    local -r unsupportedInstallLibraryOptionVersionsRange2regex="^1\.6\.[0-3]$"
    if [[ "$NEWEST_INSTALLED_IDE_VERSION" =~ $unsupportedInstallLibraryOptionVersionsRange1regex ]] || [[ "$NEWEST_INSTALLED_IDE_VERSION" =~ $unsupportedInstallLibraryOptionVersionsRange2regex ]]; then
      echo "ERROR: --install-library option is not supported by the newest version of the Arduino IDE you have installed. You must have Arduino IDE 1.6.4 or newer installed to use this function."
      return_handler "$ARDUINO_CI_SCRIPT_FAILURE_EXIT_STATUS"
    else
      local -r libraryName="$1"

      # Temporarily install the latest IDE version to use for the library installation
      install_ide_version "$NEWEST_INSTALLED_IDE_VERSION"

      # Install the library
      # shellcheck disable=SC2086
      eval \"${ARDUINO_CI_SCRIPT_APPLICATION_FOLDER}/${ARDUINO_CI_SCRIPT_IDE_INSTALLATION_FOLDER}/${ARDUINO_CI_SCRIPT_ARDUINO_COMMAND}\" --install-library "$libraryName" "$ARDUINO_CI_SCRIPT_VERBOSITY_REDIRECT"

    fi
  fi

  set +o errexit

  disable_verbosity
}

# Extract common file formats
# https://github.com/xvoland/Extract
function extract() {
  if [ -z "$1" ]; then
    # display usage if no parameters given
    echo "Usage: extract <path/file_name>.<zip|rar|bz2|gz|tar|tbz2|tgz|Z|7z|xz|ex|tar.bz2|tar.gz|tar.xz>"
    echo "       extract <path/file_name_1.ext> [path/file_name_2.ext] [path/file_name_3.ext]"
    return "$ARDUINO_CI_SCRIPT_FAILURE_EXIT_STATUS"
  else
    local filename
    for filename in "$@"; do
      if [ -f "$filename" ]; then
        case "${filename%,}" in
        *.tar.bz2 | *.tar.gz | *.tar.xz | *.tbz2 | *.tgz | *.txz | *.tar)
          tar --extract --file="$filename"
          ;;
        *.lzma)
          unlzma $ARDUINO_CI_SCRIPT_QUIET_OPTION ./"$filename"
          ;;
        *.bz2)
          bunzip2 $ARDUINO_CI_SCRIPT_QUIET_OPTION ./"$filename"
          ;;
        *.rar)
          eval unrar x -ad ./"$filename" "$ARDUINO_CI_SCRIPT_VERBOSITY_REDIRECT"
          ;;
        *.gz)
          gunzip ./"$filename"
          ;;
        *.zip)
          unzip -qq ./"$filename"
          ;;
        *.z)
          eval uncompress ./"$filename" "$ARDUINO_CI_SCRIPT_VERBOSITY_REDIRECT"
          ;;
        *.7z | *.arj | *.cab | *.chm | *.deb | *.dmg | *.iso | *.lzh | *.msi | *.rpm | *.udf | *.wim | *.xar)
          7z x ./"$filename"
          ;;
        *.xz)
          unxz $ARDUINO_CI_SCRIPT_QUIET_OPTION ./"$filename"
          ;;
        *.exe)
          cabextract $ARDUINO_CI_SCRIPT_QUIET_OPTION ./"$filename"
          ;;
        *)
          echo "extract: '$filename' - unknown archive method"
          return "$ARDUINO_CI_SCRIPT_FAILURE_EXIT_STATUS"
          ;;
        esac
      else
        echo "extract: '$filename' - file does not exist"
        return "$ARDUINO_CI_SCRIPT_FAILURE_EXIT_STATUS"
      fi
    done
  fi
}

function set_verbose_output_during_compilation() {
  enable_verbosity

  local -r verboseOutputDuringCompilation="$1"
  if [[ "$verboseOutputDuringCompilation" == "true" ]]; then
    ARDUINO_CI_SCRIPT_DETERMINED_VERBOSE_BUILD="--verbose"
  else
    ARDUINO_CI_SCRIPT_DETERMINED_VERBOSE_BUILD=""
  fi

  disable_verbosity
}

# Verify the sketch
function build_sketch() {
  enable_verbosity

  local -r sketchPath="$1"
  local -r boardID="$2"
  local -r allowFail="$3"
  local -r startIDEversion="$4"
  local -r endIDEversion="$5"

  # Set default value for buildSketchExitStatus
  local buildSketchExitStatus="$ARDUINO_CI_SCRIPT_SUCCESS_EXIT_STATUS"

  generate_ide_version_list_array "$INSTALLED_IDE_VERSION_LIST_ARRAY" "$startIDEversion" "$endIDEversion"

  if [[ "$ARDUINO_CI_SCRIPT_GENERATED_IDE_VERSION_LIST_ARRAY" == "$ARDUINO_CI_SCRIPT_IDE_VERSION_LIST_ARRAY_DECLARATION"'()' ]]; then
    echo "ERROR: The IDE version(s) specified are not installed"
    buildSketchExitStatus="$ARDUINO_CI_SCRIPT_FAILURE_EXIT_STATUS"
  else
    eval "$ARDUINO_CI_SCRIPT_GENERATED_IDE_VERSION_LIST_ARRAY"
    local IDEversion
    for IDEversion in "${IDEversionListArray[@]}"; do
      # Install the IDE
      # This must be done before searching for sketches in case the path specified is in the Arduino IDE installation folder
      install_ide_version "$IDEversion"

      # The package_index files installed by some versions of the IDE (1.6.5, 1.6.5) can cause compilation to fail for other versions (1.6.5-r4, 1.6.5-r5). Attempting to install a dummy package ensures that the correct version of those files will be installed before the sketch verification.
      # Check if the newest installed IDE version supports --install-boards
      local unsupportedInstallBoardsOptionVersionsRange1regex="^1\.5\.[0-9]$"
      local unsupportedInstallBoardsOptionVersionsRange2regex="^1\.6\.[0-3]$"
      if ! [[ "$IDEversion" =~ $unsupportedInstallBoardsOptionVersionsRange1regex ]] && ! [[ "$IDEversion" =~ $unsupportedInstallBoardsOptionVersionsRange2regex ]]; then
        # shellcheck disable=SC2086
        eval \"${ARDUINO_CI_SCRIPT_APPLICATION_FOLDER}/${ARDUINO_CI_SCRIPT_IDE_INSTALLATION_FOLDER}/${ARDUINO_CI_SCRIPT_ARDUINO_COMMAND}\" --install-boards arduino:dummy "$ARDUINO_CI_SCRIPT_VERBOSITY_REDIRECT"
        if [[ "$ARDUINO_CI_SCRIPT_VERBOSITY_LEVEL" -gt 1 ]]; then
          # The warning is printed to stdout
          echo "NOTE: The warning above \"Selected board is not available\" is caused intentionally and does not indicate a problem."
        fi
      fi

      if [[ "$sketchPath" =~ \.ino$ ]] || [[ "$sketchPath" =~ \.pde$ ]]; then
        # A sketch was specified
        if ! [[ -f "$sketchPath" ]]; then
          echo "ERROR: Specified sketch: $sketchPath doesn't exist"
          buildSketchExitStatus="$ARDUINO_CI_SCRIPT_FAILURE_EXIT_STATUS"
        elif ! build_this_sketch "$sketchPath" "$boardID" "$IDEversion" "$allowFail"; then
          # build_this_sketch returned a non-zero exit status
          buildSketchExitStatus="$ARDUINO_CI_SCRIPT_FAILURE_EXIT_STATUS"
        fi
      else
        # Search for all sketches in the path and put them in an array
        local sketchFound="false"
        # https://github.com/koalaman/shellcheck/wiki/SC2207
        declare -a sketches
        mapfile -t sketches < <(find "$sketchPath" -name "*.pde" -o -name "*.ino")
        local sketchName
        for sketchName in "${sketches[@]}"; do
          # Only verify the sketch that matches the name of the sketch folder, otherwise it will cause redundant verifications for sketches that have multiple .ino files
          local sketchFolder
          sketchFolder="$(echo "$sketchName" | rev | cut -d'/' -f 2 | rev)"
          local sketchNameWithoutPathWithExtension
          sketchNameWithoutPathWithExtension="$(echo "$sketchName" | rev | cut -d'/' -f 1 | rev)"
          local sketchNameWithoutPathWithoutExtension
          sketchNameWithoutPathWithoutExtension="${sketchNameWithoutPathWithExtension%.*}"
          if [[ "$sketchFolder" == "$sketchNameWithoutPathWithoutExtension" ]]; then
            sketchFound="true"
            if ! build_this_sketch "$sketchName" "$boardID" "$IDEversion" "$allowFail"; then
              # build_this_sketch returned a non-zero exit status
              buildSketchExitStatus="$ARDUINO_CI_SCRIPT_FAILURE_EXIT_STATUS"
            fi
          fi
        done

        if [[ "$sketchFound" == "false" ]]; then
          echo "ERROR: No valid sketches were found in the specified path"
          buildSketchExitStatus="$ARDUINO_CI_SCRIPT_FAILURE_EXIT_STATUS"
        fi
      fi
    done
  fi

  disable_verbosity

  return $buildSketchExitStatus
}

function build_this_sketch() {
  # Fold this section of output in the Travis CI build log to make it easier to read
  echo -e "travis_fold:start:build_sketch"

  local -r sketchName="$1"
  local -r boardID="$2"
  local -r IDEversion="$3"
  local -r allowFail="$4"

  # Produce a useful label for the fold in the Travis log for this function call
  echo "build_sketch $sketchName $boardID $allowFail $IDEversion"

  # Arduino IDE 1.8.0 and 1.8.1 fail to verify a sketch if the absolute path to it is not specified
  # http://stackoverflow.com/a/3915420/7059512
  local absoluteSketchName
  absoluteSketchName="$(
    cd "$(dirname "$1")"
    pwd
  )/$(basename "$1")"

  # Define a dummy value for arduinoExitStatus so that the while loop will run at least once
  local arduinoExitStatus=255
  # Retry the verification if arduino returns an exit status that indicates there may have been a temporary error not caused by a bug in the sketch or the arduino command
  while [[ $arduinoExitStatus -gt $ARDUINO_CI_SCRIPT_HIGHEST_ACCEPTABLE_ARDUINO_EXIT_STATUS && $verifyCount -le $ARDUINO_CI_SCRIPT_SKETCH_VERIFY_RETRIES ]]; do
    # Verify the sketch
    # shellcheck disable=SC2086
    "${ARDUINO_CI_SCRIPT_APPLICATION_FOLDER}/${ARDUINO_CI_SCRIPT_IDE_INSTALLATION_FOLDER}/${ARDUINO_CI_SCRIPT_ARDUINO_COMMAND}" $ARDUINO_CI_SCRIPT_DETERMINED_VERBOSE_BUILD --verify "$absoluteSketchName" --board "$boardID" 2>&1 | tr --complement --delete '[:print:]\n\t' | tr --squeeze-repeats '\n' | grep --extended-regexp --invert-match "$ARDUINO_CI_SCRIPT_ARDUINO_OUTPUT_FILTER_REGEX" | tee "$ARDUINO_CI_SCRIPT_VERIFICATION_OUTPUT_FILENAME"
    local arduinoExitStatus="${PIPESTATUS[0]}"
    local verifyCount=$((verifyCount + 1))
  done

  if [[ "$arduinoExitStatus" != "$ARDUINO_CI_SCRIPT_SUCCESS_EXIT_STATUS" ]]; then
    # Sketch verification failed
    local buildThisSketchExitStatus="$ARDUINO_CI_SCRIPT_FAILURE_EXIT_STATUS"
  else
    # Sketch verification succeeded
    local buildThisSketchExitStatus="$ARDUINO_CI_SCRIPT_SUCCESS_EXIT_STATUS"

    # Parse through the output from the sketch verification to count warnings and determine the compile size
    local warningCount=0
    local boardIssueCount=0
    local libraryIssueCount=0
    while read -r outputFileLine; do
      # Determine program storage memory usage
      local programStorageRegex="Sketch uses ([0-9,]+) *"
      if [[ "$outputFileLine" =~ $programStorageRegex ]] >/dev/null; then
        local -r programStorageWithComma=${BASH_REMATCH[1]}
      fi

      # Determine dynamic memory usage
      local dynamicMemoryRegex="Global variables use ([0-9,]+) *"
      if [[ "$outputFileLine" =~ $dynamicMemoryRegex ]] >/dev/null; then
        local -r dynamicMemoryWithComma=${BASH_REMATCH[1]}
      fi

      # Increment warning count
      local warningRegex="warning: "
      if [[ "$outputFileLine" =~ $warningRegex ]] >/dev/null; then
        warningCount=$((warningCount + 1))
      fi

      # Check for board issues
      local bootloaderMissingRegex="Bootloader file specified but missing: "
      if [[ "$outputFileLine" =~ $bootloaderMissingRegex ]] >/dev/null; then
        local boardIssue="missing bootloader"
        boardIssueCount=$((boardIssueCount + 1))
      fi

      local boardsDotTxtMissingRegex="Could not find boards.txt"
      if [[ "$outputFileLine" =~ $boardsDotTxtMissingRegex ]] >/dev/null; then
        local boardIssue="Could not find boards.txt"
        boardIssueCount=$((boardIssueCount + 1))
      fi

      local buildDotBoardNotDefinedRegex="doesn't define a 'build.board' preference"
      if [[ "$outputFileLine" =~ $buildDotBoardNotDefinedRegex ]] >/dev/null; then
        local boardIssue="doesn't define a 'build.board' preference"
        boardIssueCount=$((boardIssueCount + 1))
      fi

      # Check for library issues
      # This is the generic "invalid library" warning that doesn't specify the reason
      local invalidLibrarRegex1="Invalid library found in"
      local invalidLibrarRegex2="from library$"
      if [[ "$outputFileLine" =~ $invalidLibrarRegex1 ]] && ! [[ "$outputFileLine" =~ $invalidLibrarRegex2 ]] >/dev/null; then
        local libraryIssue="Invalid library"
        libraryIssueCount=$((libraryIssueCount + 1))
      fi

      local missingNameRegex="Invalid library found in .* Missing 'name' from library"
      if [[ "$outputFileLine" =~ $missingNameRegex ]] >/dev/null; then
        local libraryIssue="Missing 'name' from library"
        libraryIssueCount=$((libraryIssueCount + 1))
      fi

      local missingVersionRegex="Invalid library found in .* Missing 'version' from library"
      if [[ "$outputFileLine" =~ $missingVersionRegex ]] >/dev/null; then
        local libraryIssue="Missing 'version' from library"
        libraryIssueCount=$((libraryIssueCount + 1))
      fi

      local missingAuthorRegex="Invalid library found in .* Missing 'author' from library"
      if [[ "$outputFileLine" =~ $missingAuthorRegex ]] >/dev/null; then
        local libraryIssue="Missing 'author' from library"
        libraryIssueCount=$((libraryIssueCount + 1))
      fi

      local missingMaintainerRegex="Invalid library found in .* Missing 'maintainer' from library"
      if [[ "$outputFileLine" =~ $missingMaintainerRegex ]] >/dev/null; then
        local libraryIssue="Missing 'maintainer' from library"
        libraryIssueCount=$((libraryIssueCount + 1))
      fi

      local missingSentenceRegex="Invalid library found in .* Missing 'sentence' from library"
      if [[ "$outputFileLine" =~ $missingSentenceRegex ]] >/dev/null; then
        local libraryIssue="Missing 'sentence' from library"
        libraryIssueCount=$((libraryIssueCount + 1))
      fi

      local missingParagraphRegex="Invalid library found in .* Missing 'paragraph' from library"
      if [[ "$outputFileLine" =~ $missingParagraphRegex ]] >/dev/null; then
        local libraryIssue="Missing 'paragraph' from library"
        libraryIssueCount=$((libraryIssueCount + 1))
      fi

      local missingURLregex="Invalid library found in .* Missing 'url' from library"
      if [[ "$outputFileLine" =~ $missingURLregex ]] >/dev/null; then
        local libraryIssue="Missing 'url' from library"
        libraryIssueCount=$((libraryIssueCount + 1))
      fi

      local invalidVersionRegex="Invalid version found:"
      if [[ "$outputFileLine" =~ $invalidVersionRegex ]] >/dev/null; then
        local libraryIssue="Invalid version found:"
        libraryIssueCount=$((libraryIssueCount + 1))
      fi

      local invalidCategoryRegex="is not valid. Setting to 'Uncategorized'"
      if [[ "$outputFileLine" =~ $invalidCategoryRegex ]] >/dev/null; then
        local libraryIssue="Invalid category"
        libraryIssueCount=$((libraryIssueCount + 1))
      fi

      local spuriousFolderRegex="WARNING: Spurious"
      if [[ "$outputFileLine" =~ $spuriousFolderRegex ]] >/dev/null; then
        local libraryIssue="Spurious folder"
        libraryIssueCount=$((libraryIssueCount + 1))
      fi

    done <"$ARDUINO_CI_SCRIPT_VERIFICATION_OUTPUT_FILENAME"

    rm $ARDUINO_CI_SCRIPT_VERBOSITY_OPTION "$ARDUINO_CI_SCRIPT_VERIFICATION_OUTPUT_FILENAME"

    # Remove the stupid comma from the memory values if present
    local -r programStorage=${programStorageWithComma//,/}
    local -r dynamicMemory=${dynamicMemoryWithComma//,/}

    if [[ "$boardIssue" != "" && "$ARDUINO_CI_SCRIPT_TEST_BOARD" == "true" ]]; then
      # There was a board issue and board testing is enabled so fail the build
      local buildThisSketchExitStatus="$ARDUINO_CI_SCRIPT_FAILURE_EXIT_STATUS"
    fi

    if [[ "$libraryIssue" != "" && "$ARDUINO_CI_SCRIPT_TEST_LIBRARY" == "true" ]]; then
      # There was a library issue and library testing is enabled so fail the build
      local buildThisSketchExitStatus="$ARDUINO_CI_SCRIPT_FAILURE_EXIT_STATUS"
    fi
  fi

  # Add the build data to the report file
  echo "$(date -u "+%Y-%m-%d %H:%M:%S")"$'\t'"$TRAVIS_BUILD_NUMBER"$'\t'"$TRAVIS_JOB_NUMBER"$'\t'"https://travis-ci.org/${TRAVIS_REPO_SLUG}/jobs/${TRAVIS_JOB_ID}"$'\t'"$TRAVIS_EVENT_TYPE"$'\t'"$TRAVIS_ALLOW_FAILURE"$'\t'"$TRAVIS_PULL_REQUEST"$'\t'"$TRAVIS_BRANCH"$'\t'"$TRAVIS_COMMIT"$'\t'"$TRAVIS_COMMIT_RANGE"$'\t'"${TRAVIS_COMMIT_MESSAGE%%$'\n'*}"$'\t'"$sketchName"$'\t'"$boardID"$'\t'"$IDEversion"$'\t'"$programStorage"$'\t'"$dynamicMemory"$'\t'"$warningCount"$'\t'"$allowFail"$'\t'"$arduinoExitStatus"$'\t'"$boardIssueCount"$'\t'"$boardIssue"$'\t'"$libraryIssueCount"$'\t'"$libraryIssue"$'\r' >>"$ARDUINO_CI_SCRIPT_REPORT_FILE_PATH"

  # Adjust the exit status according to the allowFail setting
  if [[ "$buildThisSketchExitStatus" == "$ARDUINO_CI_SCRIPT_FAILURE_EXIT_STATUS" && ("$allowFail" == "true" || "$allowFail" == "require") ]]; then
    buildThisSketchExitStatus="$ARDUINO_CI_SCRIPT_SUCCESS_EXIT_STATUS"
  elif [[ "$buildThisSketchExitStatus" == "$ARDUINO_CI_SCRIPT_SUCCESS_EXIT_STATUS" && "$allowFail" == "require" ]]; then
    buildThisSketchExitStatus="$ARDUINO_CI_SCRIPT_FAILURE_EXIT_STATUS"
  fi

  if [[ "$buildThisSketchExitStatus" != "$ARDUINO_CI_SCRIPT_SUCCESS_EXIT_STATUS" ]]; then
    ARDUINO_CI_SCRIPT_TOTAL_SKETCH_BUILD_FAILURE_COUNT=$((ARDUINO_CI_SCRIPT_TOTAL_SKETCH_BUILD_FAILURE_COUNT + 1))
  fi
  ARDUINO_CI_SCRIPT_TOTAL_WARNING_COUNT=$((ARDUINO_CI_SCRIPT_TOTAL_WARNING_COUNT + warningCount + 0))
  ARDUINO_CI_SCRIPT_TOTAL_BOARD_ISSUE_COUNT=$((ARDUINO_CI_SCRIPT_TOTAL_BOARD_ISSUE_COUNT + boardIssueCount + 0))
  ARDUINO_CI_SCRIPT_TOTAL_LIBRARY_ISSUE_COUNT=$((ARDUINO_CI_SCRIPT_TOTAL_LIBRARY_ISSUE_COUNT + libraryIssueCount + 0))

  # End the folded section of the Travis CI build log
  echo -e "travis_fold:end:build_sketch"
  # Add a useful message to the Travis CI build log

  echo "arduino Exit Status: ${arduinoExitStatus}, Allow Failure: ${allowFail}, # Warnings: ${warningCount}, # Board Issues: ${boardIssueCount}, # Library Issues: ${libraryIssueCount}"

  return $buildThisSketchExitStatus
}

# Print the contents of the report file
function display_report() {
  enable_verbosity

  if [ -e "$ARDUINO_CI_SCRIPT_REPORT_FILE_PATH" ]; then
    echo -e '\n\n\n**************Begin Report**************\n\n\n'
    cat "$ARDUINO_CI_SCRIPT_REPORT_FILE_PATH"
    echo -e '\n\n'
    echo "Total failed sketch builds: $ARDUINO_CI_SCRIPT_TOTAL_SKETCH_BUILD_FAILURE_COUNT"
    echo "Total warnings: $ARDUINO_CI_SCRIPT_TOTAL_WARNING_COUNT"
    echo "Total board issues: $ARDUINO_CI_SCRIPT_TOTAL_BOARD_ISSUE_COUNT"
    echo "Total library issues: $ARDUINO_CI_SCRIPT_TOTAL_LIBRARY_ISSUE_COUNT"
    echo -e '\n\n'
  else
    echo "No report file available for this job"
  fi

  disable_verbosity
}

# Add the report file to a Git repository
function publish_report_to_repository() {
  enable_verbosity

  local -r token="$1"
  local -r repositoryURL="$2"
  local -r reportBranch="$3"
  local -r reportFolder="$4"
  local -r doLinkComment="$5"

  if [[ "$token" != "" ]] && [[ "$repositoryURL" != "" ]] && [[ "$reportBranch" != "" ]]; then
    if [ -e "$ARDUINO_CI_SCRIPT_REPORT_FILE_PATH" ]; then
      # Location is a repository
      if git clone --quiet --branch "$reportBranch" "$repositoryURL" "${HOME}/report-repository"; then
        # Clone was successful
        create_folder "${HOME}/report-repository/${reportFolder}"
        cp $ARDUINO_CI_SCRIPT_VERBOSITY_OPTION "$ARDUINO_CI_SCRIPT_REPORT_FILE_PATH" "${HOME}/report-repository/${reportFolder}"
        local -r previousFolder="$PWD"
        cd "${HOME}/report-repository"
        git add $ARDUINO_CI_SCRIPT_VERBOSITY_OPTION "${HOME}/report-repository/${reportFolder}/${ARDUINO_CI_SCRIPT_REPORT_FILENAME}"
        git config user.email "arduino-ci-script@nospam.me"
        git config user.name "arduino-ci-script-bot"
        # Only pushes the current branch to the corresponding remote branch that 'git pull' uses to update the current branch.
        git config push.default simple
        if [[ "$TRAVIS_TEST_RESULT" != "0" ]]; then
          local -r jobSuccessMessage="FAILED"
        else
          local -r jobSuccessMessage="SUCCESSFUL"
        fi
        # Do a pull now in case another job has finished about the same time and pushed a report after the clone happened, which would otherwise cause the push to fail. This is the last chance to pull without having to deal with a merge or rebase.
        git pull $ARDUINO_CI_SCRIPT_QUIET_OPTION
        git commit $ARDUINO_CI_SCRIPT_QUIET_OPTION $ARDUINO_CI_SCRIPT_VERBOSITY_OPTION --message="Add Travis CI job ${TRAVIS_JOB_NUMBER} report (${jobSuccessMessage})" --message="Total failed sketch builds: $ARDUINO_CI_SCRIPT_TOTAL_SKETCH_BUILD_FAILURE_COUNT" --message="Total warnings: $ARDUINO_CI_SCRIPT_TOTAL_WARNING_COUNT" --message="Total board issues: $ARDUINO_CI_SCRIPT_TOTAL_BOARD_ISSUE_COUNT" --message="Total library issues: $ARDUINO_CI_SCRIPT_TOTAL_LIBRARY_ISSUE_COUNT" --message="Job log: https://travis-ci.org/${TRAVIS_REPO_SLUG}/jobs/${TRAVIS_JOB_ID}" --message="Commit: https://github.com/${TRAVIS_REPO_SLUG}/commit/${TRAVIS_COMMIT}" --message="$TRAVIS_COMMIT_MESSAGE" --message="[skip ci]"
        local gitPushExitStatus="1"
        local pushCount=0
        while [[ "$gitPushExitStatus" != "$ARDUINO_CI_SCRIPT_SUCCESS_EXIT_STATUS" && $pushCount -le $ARDUINO_CI_SCRIPT_REPORT_PUSH_RETRIES ]]; do
          pushCount=$((pushCount + 1))
          # Do a pull now in case another job has finished about the same time and pushed a report since the last pull. This would require a merge or rebase. Rebase should be safe since the commits will be separate files.
          git pull $ARDUINO_CI_SCRIPT_QUIET_OPTION --rebase
          git push $ARDUINO_CI_SCRIPT_QUIET_OPTION $ARDUINO_CI_SCRIPT_VERBOSITY_OPTION "https://${token}@${repositoryURL#*//}"
          gitPushExitStatus="$?"
        done
        cd "$previousFolder"
        rm --recursive --force "${HOME}/report-repository"
        if [[ "$gitPushExitStatus" == "$ARDUINO_CI_SCRIPT_SUCCESS_EXIT_STATUS" ]]; then
          if [[ "$doLinkComment" == "true" ]]; then
            # Only comment if it's job 1
            local -r firstJobRegex='\.1$'
            if [[ "$TRAVIS_JOB_NUMBER" =~ $firstJobRegex ]]; then
              local reportURL
              reportURL="${repositoryURL%.*}/tree/${reportBranch}/${reportFolder}"
              comment_report_link "$token" "$reportURL"
            fi
          fi
        else
          echo "ERROR: Failed to push to remote branch."
          return_handler "$ARDUINO_CI_SCRIPT_FAILURE_EXIT_STATUS"
        fi
      else
        echo "ERROR: Failed to clone branch ${reportBranch} of repository URL ${repositoryURL}. Do they exist?"
        return_handler "$ARDUINO_CI_SCRIPT_FAILURE_EXIT_STATUS"
      fi
    else
      echo "No report file available for this job"
    fi
  else
    if [[ "$token" == "" ]]; then
      echo "ERROR: GitHub token not specified. Failed to publish build report. See https://github.com/per1234/arduino-ci-script#publishing-job-reports for instructions."
    fi
    if [[ "$repositoryURL" == "" ]]; then
      echo "ERROR: Repository URL not specified. Failed to publish build report."
    fi
    if [[ "$reportBranch" == "" ]]; then
      echo "ERROR: Repository branch not specified. Failed to publish build report."
    fi
    return_handler "$ARDUINO_CI_SCRIPT_FAILURE_EXIT_STATUS"
  fi

  disable_verbosity
}

# Add the report file to a gist
function publish_report_to_gist() {
  enable_verbosity

  local -r token="$1"
  local -r gistURL="$2"
  local -r doLinkComment="$3"

  if [[ "$token" != "" ]] && [[ "$gistURL" != "" ]]; then
    if [ -e "$ARDUINO_CI_SCRIPT_REPORT_FILE_PATH" ]; then
      # Get the gist ID from the gist URL
      local gistID
      gistID="$(echo "$gistURL" | rev | cut -d'/' -f 1 | rev)"

      # http://stackoverflow.com/a/33354920/7059512
      # Sanitize the report file content so it can be sent via a POST request without breaking the JSON
      # Remove \r (from Windows end-of-lines), replace tabs by \t, replace " by \", replace EOL by \n
      local reportContent
      reportContent=$(sed -e 's/\r//' -e's/\t/\\t/g' -e 's/"/\\"/g' "$ARDUINO_CI_SCRIPT_REPORT_FILE_PATH" | awk '{ printf($0 "\\n") }')

      # Upload the report to the Gist. I have to use the here document to avoid the "Argument list too long" error from curl with long reports. Redirect output to dev/null because it dumps the whole gist to the log
      eval curl --header "\"Authorization: token ${token}\"" --data @- "\"https://api.github.com/gists/${gistID}\"" "$ARDUINO_CI_SCRIPT_VERBOSITY_REDIRECT" <<curlDataHere
{"files":{"${ARDUINO_CI_SCRIPT_REPORT_FILENAME}":{"content": "${reportContent}"}}}
curlDataHere

      if [[ "$doLinkComment" == "true" ]]; then
        # Only comment if it's job 1
        local -r firstJobRegex='\.1$'
        if [[ "$TRAVIS_JOB_NUMBER" =~ $firstJobRegex ]]; then
          local reportURL="${gistURL}#file-${ARDUINO_CI_SCRIPT_REPORT_FILENAME//./-}"
          comment_report_link "$token" "$reportURL"
        fi
      fi
    else
      echo "No report file available for this job"
    fi
  else
    if [[ "$token" == "" ]]; then
      echo "ERROR: GitHub token not specified. Failed to publish build report. See https://github.com/per1234/arduino-ci-script#publishing-job-reports for instructions."
    fi
    if [[ "$gistURL" == "" ]]; then
      echo "ERROR: Gist URL not specified. Failed to publish build report. See https://github.com/per1234/arduino-ci-script#publishing-job-reports for instructions."
    fi
    return_handler "$ARDUINO_CI_SCRIPT_FAILURE_EXIT_STATUS"
  fi

  disable_verbosity
}

# Leave a comment on the commit with a link to the report
function comment_report_link() {
  local -r token="$1"
  local -r reportURL="$2"

  # shellcheck disable=SC1083
  # shellcheck disable=SC2026
  # shellcheck disable=SC2086
  eval curl $ARDUINO_CI_SCRIPT_VERBOSITY_OPTION --header "\"Authorization: token ${token}\"" --data \"{'\"'body'\"':'\"'Once completed, the job reports for Travis CI [build ${TRAVIS_BUILD_NUMBER}]\(https://travis-ci.org/${TRAVIS_REPO_SLUG}/builds/${TRAVIS_BUILD_ID}\) will be found at:\\n${reportURL}'\"'}\" "\"https://api.github.com/repos/${TRAVIS_REPO_SLUG}/commits/${TRAVIS_COMMIT}/comments\"" "$ARDUINO_CI_SCRIPT_VERBOSITY_REDIRECT"
  if [[ $? -ne $ARDUINO_CI_SCRIPT_SUCCESS_EXIT_STATUS ]]; then
    echo "ERROR: Failed to comment link to published report location"
  fi
}

# Deprecated because no longer necessary. Left only to maintain backwards compatibility
function check_success() {
  echo "The check_success function is no longer necessary and has been deprecated"
}

# Set the exit status to the greatest (and therefore most serious) code
function set_exit_status() {
  local -r previousExitStatus="$1"
  local -r proposedExitStatus="$2"
  if [[ $proposedExitStatus -gt $previousExitStatus ]]; then
    echo "$proposedExitStatus"
  else
    echo "$previousExitStatus"
  fi
}

ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=1
readonly ARDUINO_CI_SCRIPT_CHECK_FOLDER_NAME_INVALID_FIRST_CHARACTER_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_FOLDER_NAME_INVALID_CHARACTER_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_FOLDER_NAME_TOO_LONG_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
# The same folder name restrictions apply to libraries and sketches so this function may be used for both
function check_folder_name() {
  local -r path="$1"
  # Get the folder name from the path
  local -r folderName="${path##*/}"

  local exitStatus=$ARDUINO_CI_SCRIPT_SUCCESS_EXIT_STATUS

  # Starting folder name with a number is only supported by Arduino IDE 1.8.4 and newer
  local -r startsWithNumberRegex="^[0-9]"
  if [[ "$folderName" =~ $startsWithNumberRegex ]]; then
    echo "WARNING: Discouraged folder name: ${folderName}. Folder name beginning with a number is only supported by Arduino IDE 1.8.4 and newer."
  fi

  # Starting folder name with a - or . is not allowed
  local -r startsWithInvalidCharacterRegex="^[-.]"
  if [[ "$folderName" =~ $startsWithInvalidCharacterRegex ]]; then
    echo "ERROR: Invalid folder name: ${folderName}. Folder name beginning with a - or . is not allowed."
    exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_FOLDER_NAME_INVALID_FIRST_CHARACTER_EXIT_STATUS)
  fi

  # Allowed characters: a-z, A-Z, 0-1, -._
  local -r disallowedCharactersRegex="[^a-zA-Z0-9._-]"
  if [[ "$folderName" =~ $disallowedCharactersRegex ]]; then
    echo "ERROR: Invalid folder name: ${folderName}. Only letters, numbers, dots, dashes, and underscores are allowed."
    exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_FOLDER_NAME_INVALID_CHARACTER_EXIT_STATUS)
  fi

  # <64 characters
  if [[ ${#folderName} -gt 63 ]]; then
    echo "ERROR: Folder name $folderName exceeds the maximum of 63 characters."
    exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_FOLDER_NAME_TOO_LONG_EXIT_STATUS)
  fi
  return "$exitStatus"
}

ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=1
readonly ARDUINO_CI_SCRIPT_CHECK_SKETCH_STRUCTURE_FOLDER_DOESNT_EXIST_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_SKETCH_STRUCTURE_INCORRECT_EXTENSION_CASE_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_SKETCH_STRUCTURE_MULTIPLE_SKETCHES_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_SKETCH_STRUCTURE_SKETCH_NAME_MISMATCH_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
readonly ARDUINO_CI_SCRIPT_CHECK_SKETCH_STRUCTURE_CHECK_FOLDER_NAME_OFFSET=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
readonly ARDUINO_CI_SCRIPT_CHECK_SKETCH_STRUCTURE_FOLDER_NAME_HAS_INVALID_FIRST_CHARACTER_EXIT_STATUS=$((ARDUINO_CI_SCRIPT_CHECK_SKETCH_STRUCTURE_CHECK_FOLDER_NAME_OFFSET + ARDUINO_CI_SCRIPT_CHECK_FOLDER_NAME_INVALID_FIRST_CHARACTER_EXIT_STATUS))
readonly ARDUINO_CI_SCRIPT_CHECK_SKETCH_STRUCTURE_FOLDER_NAME_HAS_INVALID_CHARACTER_EXIT_STATUS=$((ARDUINO_CI_SCRIPT_CHECK_SKETCH_STRUCTURE_CHECK_FOLDER_NAME_OFFSET + ARDUINO_CI_SCRIPT_CHECK_FOLDER_NAME_INVALID_CHARACTER_EXIT_STATUS))
readonly ARDUINO_CI_SCRIPT_CHECK_SKETCH_STRUCTURE_FOLDER_NAME_TOO_LONG_EXIT_STATUS=$((ARDUINO_CI_SCRIPT_CHECK_SKETCH_STRUCTURE_CHECK_FOLDER_NAME_OFFSET + ARDUINO_CI_SCRIPT_CHECK_FOLDER_NAME_TOO_LONG_EXIT_STATUS))
function check_sketch_structure() {
  local -r searchPath="$1"
  # Replace backslashes with slashes
  local -r searchPathWithSlashes="${searchPath//\\//}"
  # Remove trailing slash
  local -r normalizedSearchPath="${searchPathWithSlashes%/}"

  local exitStatus=$ARDUINO_CI_SCRIPT_SUCCESS_EXIT_STATUS

  # Check whether folder exists
  if [[ ! -d "$normalizedSearchPath" ]]; then
    echo "ERROR: ${searchPath}: Folder doesn't exist."
    return $ARDUINO_CI_SCRIPT_CHECK_SKETCH_STRUCTURE_FOLDER_DOESNT_EXIST_EXIT_STATUS
  fi

  # find all folders that contain a sketch file
  while read -r sketchPath; do
    # The while loop always runs once, even when no sketches were found in $normalizedSearchPath
    if [[ "$sketchPath" == "" ]]; then
      continue
    fi

    local folderNameMismatch=true
    local primarySketchFound=false
    # find all sketch files in $sketchPath
    while read -r sketchFilePath; do
      # Check for sketches that use the obsolete .pde extension
      if [[ "${sketchFilePath: -4}" == ".pde" ]]; then
        echo "WARNING: ${sketchFilePath}: Uses .pde extension. For Arduino sketches, it's recommended to use the .ino extension instead. If this is a Processing sketch then .pde is the correct extension."
      fi

      # Check for sketches with incorrect extension case
      if [[ "${sketchFilePath: -4}" != ".ino" && "${sketchFilePath: -4}" != ".pde" ]]; then
        echo "ERROR: ${sketchFilePath}: Incorrect extension case. This causes it to not be recognized on a filename case-sensitive OS such as Linux."
        exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_SKETCH_STRUCTURE_INCORRECT_EXTENSION_CASE_EXIT_STATUS)
      fi

      # Check for folder name mismatch
      local sketchFileFolderName="${sketchPath##*/}"
      local sketchFilenameWithExtension="${sketchFilePath##*/}"
      local sketchFilenameWithoutExtension="${sketchFilenameWithExtension%.*}"
      if [[ "$sketchFileFolderName" == "$sketchFilenameWithoutExtension" ]]; then
        # Sketch file found that matches the folder name
        folderNameMismatch=false
        # don't return because this represents a non-error and so the folder should continue to be checked for different errors
      fi

      # Check for multiple sketches in folder
      if grep --quiet --regexp='void  *setup *( *)' "$sketchFilePath"; then
        if [[ "$primarySketchFound" == true ]]; then
          # A primary sketch file was previously found in this folder
          echo "ERROR: ${sketchPath}: Multiple sketches found in the same folder."
          exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_SKETCH_STRUCTURE_MULTIPLE_SKETCHES_EXIT_STATUS)
        fi
        local primarySketchFound=true
      fi
    done < <(find "$sketchPath" -maxdepth 1 -type f \( -iname '*.ino' -or -iname '*.pde' \) -print)
    if [[ "$folderNameMismatch" == true ]]; then
      echo "ERROR: ${sketchPath}: Folder name does not match the sketch filename."
      exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_SKETCH_STRUCTURE_SKETCH_NAME_MISMATCH_EXIT_STATUS)
    fi

    # Check if sketch name is valid
    check_folder_name "$sketchPath"
    local checkFolderNameExitStatus=$?
    if [[ $checkFolderNameExitStatus -ne $ARDUINO_CI_SCRIPT_SUCCESS_EXIT_STATUS ]]; then
      exitStatus=$(set_exit_status "$exitStatus" $((ARDUINO_CI_SCRIPT_CHECK_SKETCH_STRUCTURE_CHECK_FOLDER_NAME_OFFSET + checkFolderNameExitStatus)))
    fi

  done <<<"$(find "$normalizedSearchPath" -type f \( -iname '*.ino' -or -iname '*.pde' \) -printf '%h\n' | sort --unique)"
  return "$exitStatus"
}

# https://github.com/arduino/Arduino/wiki/Arduino-IDE-1.5:-Library-specification
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=1
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_INCORRECT_EXTRAS_FOLDER_NAME_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_INCORRECT_EXAMPLES_FOLDER_NAME_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_STRAY_LIBRARY_PROPERTIES_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_STRAY_KEYWORDS_TXT_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_STRAY_SKETCH_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_SPURIOUS_DOT_FOLDER_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_FOLDER_DOESNT_EXIST_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_INCORRECT_SRC_FOLDER_CASE_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_LIBRARY_NOT_FOUND_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_CHECK_FOLDER_NAME_OFFSET=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_FOLDER_NAME_HAS_INVALID_FIRST_CHARACTER_EXIT_STATUS=$((ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_CHECK_FOLDER_NAME_OFFSET + ARDUINO_CI_SCRIPT_CHECK_FOLDER_NAME_INVALID_FIRST_CHARACTER_EXIT_STATUS))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_FOLDER_NAME_HAS_INVALID_CHARACTER_EXIT_STATUS=$((ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_CHECK_FOLDER_NAME_OFFSET + ARDUINO_CI_SCRIPT_CHECK_FOLDER_NAME_INVALID_CHARACTER_EXIT_STATUS))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_FOLDER_NAME_TOO_LONG_EXIT_STATUS=$((ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_CHECK_FOLDER_NAME_OFFSET + ARDUINO_CI_SCRIPT_CHECK_FOLDER_NAME_TOO_LONG_EXIT_STATUS))
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_FOLDER_NAME_TOO_LONG_EXIT_STATUS + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_SRC_AND_UTILITY_FOLDERS_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_CHECK_SKETCH_STRUCTURE_OFFSET=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_SKETCH_FOLDER_DOESNT_EXIST_EXIT_STATUS=$((ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_CHECK_SKETCH_STRUCTURE_OFFSET + ARDUINO_CI_SCRIPT_CHECK_SKETCH_STRUCTURE_FOLDER_DOESNT_EXIST_EXIT_STATUS))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_INCORRECT_SKETCH_EXTENSION_CASE_EXIT_STATUS=$((ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_CHECK_SKETCH_STRUCTURE_OFFSET + ARDUINO_CI_SCRIPT_CHECK_SKETCH_STRUCTURE_INCORRECT_EXTENSION_CASE_EXIT_STATUS))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_MULTIPLE_SKETCHES_EXIT_STATUS=$((ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_CHECK_SKETCH_STRUCTURE_OFFSET + ARDUINO_CI_SCRIPT_CHECK_SKETCH_STRUCTURE_MULTIPLE_SKETCHES_EXIT_STATUS))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_SKETCH_NAME_MISMATCH_EXIT_STATUS=$((ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_CHECK_SKETCH_STRUCTURE_OFFSET + ARDUINO_CI_SCRIPT_CHECK_SKETCH_STRUCTURE_SKETCH_NAME_MISMATCH_EXIT_STATUS))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_SKETCH_FOLDER_NAME_HAS_INVALID_FIRST_CHARACTER_EXIT_STATUS=$((ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_CHECK_SKETCH_STRUCTURE_OFFSET + ARDUINO_CI_SCRIPT_CHECK_SKETCH_STRUCTURE_FOLDER_NAME_HAS_INVALID_FIRST_CHARACTER_EXIT_STATUS))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_SKETCH_FOLDER_NAME_HAS_INVALID_CHARACTER_EXIT_STATUS=$((ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_CHECK_SKETCH_STRUCTURE_OFFSET + ARDUINO_CI_SCRIPT_CHECK_SKETCH_STRUCTURE_FOLDER_NAME_HAS_INVALID_CHARACTER_EXIT_STATUS))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_SKETCH_FOLDER_NAME_TOO_LONG_EXIT_STATUS=$((ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_CHECK_SKETCH_STRUCTURE_OFFSET + ARDUINO_CI_SCRIPT_CHECK_SKETCH_STRUCTURE_FOLDER_NAME_TOO_LONG_EXIT_STATUS))
function check_library_structure() {
  local -r basePath="$1"
  local depth="$2"
  if [[ "$depth" == "" ]]; then
    # Set default search depth
    depth=0
  fi

  # Replace backslashes with slashes
  local -r basePathWithSlashes="${basePath//\\//}"
  # Remove trailing slash
  local -r normalizedBasePath="${basePathWithSlashes%/}"

  local exitStatus=$ARDUINO_CI_SCRIPT_SUCCESS_EXIT_STATUS

  # Check whether folder exists
  if [[ ! -d "$normalizedBasePath" ]]; then
    echo "ERROR: ${basePath}: Folder doesn't exist."
    return $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_FOLDER_DOESNT_EXIST_EXIT_STATUS
  fi

  while read -r normalizedLibraryPath; do

    # Check for valid 1.0 or 1.5 format
    # The Arduino IDE only recognizes libraries if they have at least one file with a header extension of the correct case (even on Windows)
    if [[ $(find "$normalizedLibraryPath" -maxdepth 1 -type f \( -name '*.h' -or -name '*.hh' -or -name '*.hpp' \)) ]]; then
      # 1.0 format library, do nothing (this if just makes the logic more simple)
      local onePointFiveFormat=false
    elif [[ $(find "$normalizedLibraryPath" -maxdepth 1 \( -type f -and -name 'library.properties' \)) && $(find "$normalizedLibraryPath" -maxdepth 1 -type d -and -iregex '^.*/src$') ]]; then
      # 1.5 format library
      if [[ ! $(find "$normalizedLibraryPath" -maxdepth 1 -type d -and -name 'src') ]]; then
        echo "ERROR: ${normalizedLibraryPath}: Is a 1.5 format library with incorrect case in src subfolder name, which causes library to not be recognized on a filename case-sensitive OS such as Linux."
        exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_INCORRECT_SRC_FOLDER_CASE_EXIT_STATUS)
        # incorrect src folder case messes up some of the later checks so skip everything else
        continue
      elif [[ $(find "${normalizedLibraryPath}/src" -maxdepth 1 -type f \( -name '*.h' -or -name '*.hh' -or -name '*.hpp' \)) ]]; then
        local onePointFiveFormat=true
      fi
    else
      echo "ERROR: ${normalizedLibraryPath}: No valid library found."
      exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_LIBRARY_NOT_FOUND_EXIT_STATUS)
      continue
    fi

    # Check if folder name is valid
    check_folder_name "$normalizedLibraryPath"
    local checkFolderNameExitStatus=$?
    if [[ $checkFolderNameExitStatus -ne $ARDUINO_CI_SCRIPT_SUCCESS_EXIT_STATUS ]]; then
      exitStatus=$(set_exit_status "$exitStatus" $((ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_CHECK_FOLDER_NAME_OFFSET + checkFolderNameExitStatus)))
    fi

    # Check for spurious dot folders not on the whitelist: https://github.com/arduino/arduino-builder/blob/master/utils/utils.go#L155
    while read -r spuriousDotFolderPath; do
      # The while loop always runs once, even when no file is found
      if [[ "$spuriousDotFolderPath" == "" ]]; then
        continue
      fi

      echo "ERROR: ${spuriousDotFolderPath}: Causes the Arduino IDE to display a spurious folder warning."
      exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_SPURIOUS_DOT_FOLDER_EXIT_STATUS)
    done <<<"$(find "$normalizedLibraryPath" -maxdepth 1 -type d -regex '^.*/\..*$' -not -name '.git' -not -name '.github' -not -name '.svn' -not -name '.hg' -not -name '.bzr' -not -name '.vscode')"

    # Check for incorrect spelling of extras folder
    if [[ $(find "$normalizedLibraryPath" -maxdepth 1 -type d -and -iregex '^.*/extras?$') && ! $(find "$normalizedLibraryPath" -maxdepth 1 -type d -and -name 'extras') ]]; then
      #if [[ -d "$normalizedLibraryPath/extra" || -d "$normalizedLibraryPath/Extras" || -d "$normalizedLibraryPath/EXTRAS" ]] && [[ ! -d "$normalizedLibraryPath/extras" ]]; then
      echo "ERROR: ${normalizedLibraryPath}: Incorrectly spelled extras folder name. It should be spelled exactly \"extras\". See: https://github.com/arduino/Arduino/wiki/Arduino-IDE-1.5:-Library-specification#extra-documentation"
      exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_INCORRECT_EXTRAS_FOLDER_NAME_EXIT_STATUS)
    fi

    # Check for incorrect spelling of examples folder
    if [[ $(find "$normalizedLibraryPath" -maxdepth 1 -type d -and -iregex '^.*/examples?$') && ! $(find "$normalizedLibraryPath" -maxdepth 1 -type d -and -name 'examples') ]]; then
      echo "ERROR: ${normalizedLibraryPath}: Incorrect examples folder name. It should be spelled exactly \"examples\". See: https://github.com/arduino/Arduino/wiki/Arduino-IDE-1.5:-Library-specification#library-examples"
      exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_INCORRECT_EXAMPLES_FOLDER_NAME_EXIT_STATUS)
    fi

    # Check for 1.5 format with src and utility folders in library root
    if [[ "$onePointFiveFormat" == true && $(find "$normalizedLibraryPath" -maxdepth 1 -type d -and -name 'utility') ]]; then
      echo "ERROR: ${normalizedLibraryPath}: 1.5 format library with src and utility folders in library root. The utility folder should be moved under the src folder. See: https://github.com/arduino/Arduino/wiki/Arduino-IDE-1.5:-Library-specification#source-code"
      exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_SRC_AND_UTILITY_FOLDERS_EXIT_STATUS)
    fi

    # Check for inconsequential missing library.properties file
    if ! [[ -e "$normalizedLibraryPath/library.properties" ]]; then
      echo "WARNING: ${normalizedLibraryPath}: Missing library.properties file. While not required, it's recommended to add this file to provide helpful metadata. See: https://github.com/arduino/Arduino/wiki/Arduino-IDE-1.5:-Library-specification#library-metadata"
    fi

    # Check for library.properties files in the src folder
    if [[ "$onePointFiveFormat" == true ]]; then
      while read -r normalizedLibraryPropertiesPath; do
        # The while loop always runs once, even when no file is found
        if [[ "$normalizedLibraryPropertiesPath" == "" ]]; then
          continue
        fi

        echo "ERROR: ${normalizedLibraryPropertiesPath}: Stray file. library.properties should be located in the library root folder."
        exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_STRAY_LIBRARY_PROPERTIES_EXIT_STATUS)
      done <<<"$(find "$normalizedLibraryPath/src" -maxdepth 1 -type f -iname 'library.properties')"
    fi

    # Check for missing keywords.txt file
    if ! [[ -e "$normalizedLibraryPath/keywords.txt" ]]; then
      echo "WARNING: ${normalizedLibraryPath}: Missing keywords.txt file. While not required, it's recommended to add this file to provide keyword highlighting in the Arduino IDE. See: https://github.com/arduino/Arduino/wiki/Arduino-IDE-1.5:-Library-specification#keywords"
    fi

    # Check for keywords.txt files in the src folder
    if [[ "$onePointFiveFormat" == true ]]; then
      while read -r keywordsTxtPath; do
        # The while loop always runs once, even when no file is found
        if [[ "$keywordsTxtPath" == "" ]]; then
          continue
        fi

        echo "ERROR: ${keywordsTxtPath}: Stray file. keywords.txt should be located in the library root folder."
        exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_STRAY_KEYWORDS_TXT_EXIT_STATUS)
      done <<<"$(find "$normalizedLibraryPath/src" -maxdepth 1 -type f -iname 'keywords.txt')"
    fi

    # Check for sketch files outside of the src or extras folders
    if [[ $(find "$normalizedLibraryPath" -maxdepth 1 -path './examples' -prune -or -path './extras' -prune -or \( -type f -and \( -iname '*.ino' -or -iname '*.pde' \) \)) ]]; then
      echo "ERROR: ${normalizedLibraryPath}: Sketch files found outside the examples and extras folders."
      exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_STRAY_SKETCH_EXIT_STATUS)
    fi

    # Run check_sketch_structure() on examples and extras folders
    if [[ -d "${normalizedLibraryPath}/examples" ]]; then
      check_sketch_structure "${normalizedLibraryPath}/examples"
      local checkExamplesSketchStructureExitStatus=$?
      if [[ $checkExamplesSketchStructureExitStatus -ne $ARDUINO_CI_SCRIPT_SUCCESS_EXIT_STATUS ]]; then
        exitStatus=$(set_exit_status "$exitStatus" $((ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_CHECK_SKETCH_STRUCTURE_OFFSET + checkExamplesSketchStructureExitStatus)))
      fi
    fi
    if [[ -d "${normalizedLibraryPath}/extras" ]]; then
      check_sketch_structure "${normalizedLibraryPath}/extras"
      local checkExtrasSketchStructureExitStatus=$?
      if [[ $checkExtrasSketchStructureExitStatus -ne $ARDUINO_CI_SCRIPT_SUCCESS_EXIT_STATUS ]]; then
        exitStatus=$(set_exit_status "$exitStatus" $((ARDUINO_CI_SCRIPT_CHECK_LIBRARY_STRUCTURE_CHECK_SKETCH_STRUCTURE_OFFSET + checkExtrasSketchStructureExitStatus)))
      fi
    fi

  done <<<"$(find "$normalizedBasePath" -mindepth "$depth" -maxdepth "$depth" -type d)"

  return "$exitStatus"
}

function check_architecture_alias() {
  local -r architecture="$1"
  local -r architectureAliasRegex="$2"
  local -r architecturesList="$3"
  local -r correctArchitecture="$4"
  local -r normalizedLibraryPropertiesPath="$5"
  if [[ "$architecture" =~ $architectureAliasRegex ]]; then
    local correctArchitectureFound=false
    for rawArchitecture2 in $architecturesList; do
      # The Arduino IDE ignores leading or trailing whitespace on architectures
      # Strip leading whitespace
      local architecture2="${rawArchitecture2#"${rawArchitecture2%%[![:space:]]*}"}"
      # Strip trailing whitespace
      architecture2="${architecture2%"${architecture2##*[![:space:]]}"}"
      if [[ "$architecture2" == "$correctArchitecture" ]]; then
        correctArchitectureFound=true
      fi
    done
    if [[ "$correctArchitectureFound" == false ]]; then
      echo "ERROR: ${normalizedLibraryPropertiesPath}/library.properties: architectures field contains an invalid architecture: ${architecture}. Note: architecture values are case-sensitive."
      return "$ARDUINO_CI_SCRIPT_FAILURE_EXIT_STATUS"
    fi
  fi
  return "$ARDUINO_CI_SCRIPT_SUCCESS_EXIT_STATUS"
}

function get_library_properties_field_value() {
  local -r libraryProperties="$1"
  local -r fieldName="$2"

  local line
  line=$(grep --regexp='^[[:space:]]*'"$fieldName"'[[:space:]]*=' <<<"$libraryProperties" | tail --lines=1)
  # Strip leading whitespace
  line="${line#"${line%%[![:space:]]*}"}"
  # Strip field name
  line="${line#$fieldName}"
  # Strip leading whitespace
  line="${line#"${line%%[![:space:]]*}"}"
  # Strip equals sign
  line="${line#=}"
  # Strip leading whitespace
  line="${line#"${line%%[![:space:]]*}"}"
  # Strip trailing whitespace
  line="${line%"${line##*[![:space:]]}"}"
  echo "$line"
}

function check_field_name_case() {
  local -r libraryProperties="$1"
  local -r fieldName="$2"
  local -r normalizedLibraryPropertiesPath="$3"

  if ! grep --quiet --regexp='^[[:space:]]*'"$fieldName"'[[:space:]]*=' <<<"$libraryProperties"; then
    if grep --quiet --ignore-case --regexp='^[[:space:]]*'"$fieldName"'[[:space:]]*=' <<<"$libraryProperties"; then
      echo "ERROR: ${normalizedLibraryPropertiesPath}/library.properties: $fieldName field name has incorrect case. It must be spelled exactly \"${fieldName}\". See https://github.com/arduino/Arduino/wiki/Arduino-IDE-1.5:-Library-specification#libraryproperties-file-format"
      return "$ARDUINO_CI_SCRIPT_FAILURE_EXIT_STATUS"
    fi
  fi
  return "$ARDUINO_CI_SCRIPT_SUCCESS_EXIT_STATUS"
}

# https://github.com/arduino/Arduino/wiki/Arduino-IDE-1.5:-Library-specification#libraryproperties-file-format
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=1
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_REDUNDANT_PARAGRAPH_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_BLANK_NAME=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_INVALID_ARCHITECTURE_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_ARCHITECTURES_MISSPELLED_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_ARCHITECTURES_EMPTY_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_FOLDER_DOESNT_EXIST_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_MISSPELLED_FILENAME_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_INCORRECT_FILENAME_CASE_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_MISSING_NAME_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_MISSING_VERSION_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_MISSING_AUTHOR_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_MISSING_MAINTAINER_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_MISSING_SENTENCE_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_MISSING_PARAGRAPH_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_MISSING_CATEGORY_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_MISSING_URL_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_INVALID_LINE_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_INVALID_VERSION_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_INVALID_CATEGORY_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_URL_BLANK_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_URL_MISSING_SCHEME_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_DEAD_URL_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_INCLUDES_MISSPELLED_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_DOT_A_LINKAGE_MISSPELLED_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_PRECOMPILED_MISSPELLED_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_LDFLAGS_MISSPELLED_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_EMPTY_INCLUDES_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
function check_library_properties() {
  local -r libraryPropertiesSearchPath="$1"
  local maximumSearchDepth="$2"
  if [[ "$maximumSearchDepth" == "" ]]; then
    # Set default search depth
    maximumSearchDepth=0
  fi

  local exitStatus=$ARDUINO_CI_SCRIPT_SUCCESS_EXIT_STATUS

  # Replace backslashes with slashes
  local -r libraryPropertiesSearchPathWithSlashes="${libraryPropertiesSearchPath//\\//}"
  local -r libraryPropertiesRegex='[lL][iI][bB][rR][aA][rR][yY]\.[pP][rR][oO][pP][eE][rR][tT][iI][eE][sS]$'
  if [[ ! -d "$libraryPropertiesSearchPathWithSlashes" && "$libraryPropertiesSearchPathWithSlashes" =~ $libraryPropertiesRegex ]]; then
    # Path contains the filename but we only want the folder
    local -r libraryPropertiesSearchPathWithoutFile="${libraryPropertiesSearchPathWithSlashes::-18}"
  else
    local -r libraryPropertiesSearchPathWithoutFile="$libraryPropertiesSearchPathWithSlashes"
  fi
  # Remove trailing slash
  local -r normalizedLibraryPropertiesSearchPath="${libraryPropertiesSearchPathWithoutFile%/}"

  # Check whether folder exists
  if [[ ! -d "$normalizedLibraryPropertiesSearchPath" ]]; then
    echo "ERROR: ${normalizedLibraryPropertiesSearchPath}: Folder doesn't exist."
    return $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_FOLDER_DOESNT_EXIST_EXIT_STATUS
  fi

  while read -r normalizedLibraryPropertiesPath; do

    # Check for misspelled library.properties filename
    if [[ "$(find "$normalizedLibraryPropertiesPath" -type f -regextype posix-extended -iregex '.*/librar(y|(ies))\.property')" || "$(find "$normalizedLibraryPropertiesPath" -type f -regextype posix-extended -iregex '.*/libraries\.propert(y|(ies))')" || "$(find "$normalizedLibraryPropertiesPath" -type f -regextype posix-extended -iregex '.*/librar(y|(ies))\.propert(y|(ies))\.te?xt')" ]]; then
      echo "ERROR: ${normalizedLibraryPropertiesPath}: Incorrectly spelled library.properties file. It must be spelled exactly \"library.properties\"."
      exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_MISSPELLED_FILENAME_EXIT_STATUS)
    fi

    # Check for incorrect filename case
    local libraryPropertiesFound=false
    while read -r foundLibraryPropertiesPath; do
      # The while loop always runs once, even if no file was found
      if [[ "$foundLibraryPropertiesPath" == "" ]]; then
        continue
      fi

      if [[ "${foundLibraryPropertiesPath: -18}" == 'library.properties' ]]; then
        libraryPropertiesFound=true
      else
        echo "ERROR: ${foundLibraryPropertiesPath}: Incorrect filename case. This causes it to not be recognized on a filename case-sensitive OS such as Linux. It must be exactly \"library.properties\"."
        exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_INCORRECT_FILENAME_CASE_EXIT_STATUS)
      fi
    done <<<"$(find "$normalizedLibraryPropertiesPath" -maxdepth 1 -type f -iname 'library.properties')"

    # Check whether the folder contains a library.properties file
    if [[ "$libraryPropertiesFound" == false ]]; then
      # no point in doing any more checks on this folder
      continue
    fi

    # Get rid of the CRs
    local libraryProperties
    libraryProperties=$(tr "\r" "\n" <"$normalizedLibraryPropertiesPath/library.properties")

    # Check for missing name field
    if ! grep --quiet --regexp='^[[:space:]]*name[[:space:]]*=' <<<"$libraryProperties"; then
      echo "ERROR: ${normalizedLibraryPropertiesPath}/library.properties: Missing required name field. See https://github.com/arduino/Arduino/wiki/Arduino-IDE-1.5:-Library-specification#libraryproperties-file-format"
      exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_MISSING_NAME_EXIT_STATUS)
    else
      # Check for characters in the name value disallowed by the Library Manager indexer
      local nameValue
      nameValue="$(get_library_properties_field_value "$libraryProperties" 'name')"

      # Check for blank name value
      if [[ "$nameValue" == "" ]]; then
        echo "ERROR: ${normalizedLibraryPropertiesPath}/library.properties: Has an undefined name field."
        exitStatus=$(set_exit_status "$exitStatus" $((ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_BLANK_NAME + checkFolderNameExitStatus)))
      else
        # Check for invalid name value
        # Library Manager installs libraries to a folder that is the name field value with any spaces replaced with _
        local libraryManagerFolderName="${nameValue// /_}"
        check_folder_name "$libraryManagerFolderName"
        local checkFolderNameExitStatus=$?
        if [[ $checkFolderNameExitStatus -ne $ARDUINO_CI_SCRIPT_SUCCESS_EXIT_STATUS ]]; then
          echo "WARNING: ${normalizedLibraryPropertiesPath}/library.properties: Name value $nameValue does not meet the requirements of the Arduino Library Manager indexer. See: https://github.com/arduino/Arduino/wiki/Arduino-IDE-1.5:-Library-specification#libraryproperties-file-format"
        fi
      fi
    fi

    # Check for missing version field
    if ! grep --quiet --regexp='^[[:space:]]*version[[:space:]]*=' <<<"$libraryProperties"; then
      echo "ERROR: ${normalizedLibraryPropertiesPath}/library.properties: Missing required version field. See https://github.com/arduino/Arduino/wiki/Arduino-IDE-1.5:-Library-specification#libraryproperties-file-format"
      exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_MISSING_VERSION_EXIT_STATUS)
    else
      # Check for invalid version
      if ! grep --quiet --extended-regexp --regexp='^[[:space:]]*version[[:space:]]*=[[:space:]]*(((([1-9][0-9]*)|0)\.){0,2})(([1-9][0-9]*)|0)(-(([a-zA-Z0-9-]*\.)*)([a-zA-Z0-9-]+))?(\+(([a-zA-Z0-9-]*\.)*)([a-zA-Z0-9-]+))?[[:space:]]*$' <<<"$libraryProperties"; then
        echo "ERROR: ${normalizedLibraryPropertiesPath}/library.properties: Invalid version value. Follow the semver specification: https://semver.org"
        exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_INVALID_VERSION_EXIT_STATUS)
      fi
    fi

    # Check for missing author field
    if ! grep --quiet --regexp='^[[:space:]]*author[[:space:]]*=' <<<"$libraryProperties"; then
      echo "ERROR: ${normalizedLibraryPropertiesPath}/library.properties: Missing required author field. See https://github.com/arduino/Arduino/wiki/Arduino-IDE-1.5:-Library-specification#libraryproperties-file-format"
      exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_MISSING_AUTHOR_EXIT_STATUS)
    fi

    # Check for missing maintainer field
    if ! grep --quiet --regexp='^[[:space:]]*maintainer[[:space:]]*=' <<<"$libraryProperties"; then
      if grep --quiet --regexp='^[[:space:]]*email[[:space:]]*=' <<<"$libraryProperties"; then
        echo "WARNING: ${normalizedLibraryPropertiesPath}/library.properties: Use of undocumented email field. It's recommended to use the maintainer field instead, per the Arduino Library Specification. See https://github.com/arduino/Arduino/wiki/Arduino-IDE-1.5:-Library-specification#libraryproperties-file-format"
      else
        echo "ERROR: ${normalizedLibraryPropertiesPath}/library.properties: Missing required maintainer field. See https://github.com/arduino/Arduino/wiki/Arduino-IDE-1.5:-Library-specification#libraryproperties-file-format"
        exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_MISSING_MAINTAINER_EXIT_STATUS)
      fi
    fi

    # Check for missing sentence field
    if ! grep --quiet --regexp='^[[:space:]]*sentence[[:space:]]*=' <<<"$libraryProperties"; then
      echo "ERROR: ${normalizedLibraryPropertiesPath}/library.properties: Missing required sentence field. See https://github.com/arduino/Arduino/wiki/Arduino-IDE-1.5:-Library-specification#libraryproperties-file-format"
      exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_MISSING_SENTENCE_EXIT_STATUS)
    fi

    # Check for missing paragraph field
    if ! grep --quiet --regexp='^[[:space:]]*paragraph[[:space:]]*=' <<<"$libraryProperties"; then
      echo "ERROR: ${normalizedLibraryPropertiesPath}/library.properties: Missing required paragraph field. See https://github.com/arduino/Arduino/wiki/Arduino-IDE-1.5:-Library-specification#libraryproperties-file-format"
      exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_MISSING_PARAGRAPH_EXIT_STATUS)
    else
      # Check for repeat of sentence in paragraph
      local sentenceValue
      sentenceValue="$(get_library_properties_field_value "$libraryProperties" 'sentence')"
      local sentenceValueNoPunctuation=${sentenceValue%%\.}
      if [[ "$sentenceValueNoPunctuation" != "" ]]; then
        local paragraphValue
        paragraphValue="$(get_library_properties_field_value "$libraryProperties" 'paragraph')"
        if [[ "$paragraphValue" == "$sentenceValueNoPunctuation"* ]]; then
          echo "ERROR: ${normalizedLibraryPropertiesPath}/library.properties: paragraph value repeats the sentence. These strings are displayed one after the other in Library Manager so there is no point in redundancy."
          exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_REDUNDANT_PARAGRAPH_EXIT_STATUS)
        fi
      fi
    fi

    # Check for missing category field
    if ! grep --quiet --regexp='^[[:space:]]*category[[:space:]]*=' <<<"$libraryProperties"; then
      echo "ERROR: ${normalizedLibraryPropertiesPath}/library.properties: Missing category field. This results in an invalid category warning. See https://github.com/arduino/Arduino/wiki/Arduino-IDE-1.5:-Library-specification#libraryproperties-file-format"
      exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_MISSING_CATEGORY_EXIT_STATUS)
    else
      # Check for invalid category
      if ! grep --quiet --extended-regexp --regexp='^[[:space:]]*category[[:space:]]*=[[:space:]]*((Display)|(Communication)|(Signal Input/Output)|(Sensors)|(Device Control)|(Timing)|(Data Storage)|(Data Processing)|(Other))[[:space:]]*$' <<<"$libraryProperties"; then
        if grep --quiet --regexp='^[[:space:]]*category[[:space:]]*=[[:space:]]*Uncategorized[[:space:]]*$' <<<"$libraryProperties"; then
          echo "WARNING: ${normalizedLibraryPropertiesPath}/library.properties: Category \"Uncategorized\" is not recommended. Please chose an appropriate category from https://github.com/arduino/Arduino/wiki/Arduino-IDE-1.5:-Library-specification#libraryproperties-file-format"
        else
          echo "ERROR: ${normalizedLibraryPropertiesPath}/library.properties: Invalid category value. Please chose a valid category from https://github.com/arduino/Arduino/wiki/Arduino-IDE-1.5:-Library-specification#libraryproperties-file-format"
          exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_INVALID_CATEGORY_EXIT_STATUS)
        fi
      fi
    fi

    # Check for missing url field
    if ! grep --quiet --regexp='^[[:space:]]*url[[:space:]]*=' <<<"$libraryProperties"; then
      echo "ERROR: ${normalizedLibraryPropertiesPath}/library.properties: Missing required url field. See https://github.com/arduino/Arduino/wiki/Arduino-IDE-1.5:-Library-specification#libraryproperties-file-format"
      exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_MISSING_URL_EXIT_STATUS)
    else
      # url field checks
      local urlValue
      urlValue="$(get_library_properties_field_value "$libraryProperties" 'url')"

      # Check for blank url value
      if [[ "$urlValue" == "" ]]; then
        echo "ERROR: ${normalizedLibraryPropertiesPath}/library.properties: Undefined url field. This results in a \"More info\" link in Library Manager that looks clickable but is not."
        exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_URL_BLANK_EXIT_STATUS)
      else
        # Check for missing scheme on url value
        local schemeRegex='^(http://)|(https://)'
        if ! [[ "$urlValue" =~ $schemeRegex ]]; then
          echo "ERROR: ${normalizedLibraryPropertiesPath}/library.properties: url value $urlValue is missing the scheme (e.g. https://). URL scheme must be specified for Library Manager's \"More info\" link to be clickable."
          exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_URL_MISSING_SCHEME_EXIT_STATUS)
        fi

        # Check for dead url value
        local urlStatus
        urlStatus=$(curl --location --request GET --output /dev/null --silent --head --write-out '%{http_code}' "$urlValue")
        local errorStatusRegex='^[045]'
        if [[ "$urlStatus" =~ $errorStatusRegex ]]; then
          echo "ERROR: ${normalizedLibraryPropertiesPath}/library.properties: url value $urlValue returned error status $urlStatus."
          exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_DEAD_URL_EXIT_STATUS)
        fi
      fi
    fi

    # Check for architectures field name case
    if ! check_field_name_case "$libraryProperties" 'architectures' "$normalizedLibraryPropertiesPath"; then
      exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_ARCHITECTURES_MISSPELLED_EXIT_STATUS)
    fi

    # Check for misspelled architectures field
    if grep --quiet --regexp='^[[:space:]]*architecture[[:space:]]*=' <<<"$libraryProperties"; then
      echo "ERROR: ${normalizedLibraryPropertiesPath}/library.properties: Misspelled architectures field name \"architecture\"."
      exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_ARCHITECTURES_MISSPELLED_EXIT_STATUS)
    fi

    # Check for missing architectures field
    if ! grep --quiet --regexp='^[[:space:]]*architectures[[:space:]]*=' <<<"$libraryProperties"; then
      echo "WARNING: ${normalizedLibraryPropertiesPath}/library.properties: Missing architectures field. This causes the Arduino IDE to assume the library is compatible with all architectures (*). See https://github.com/arduino/Arduino/wiki/Arduino-IDE-1.5:-Library-specification#libraryproperties-file-format"
    else
      local architecturesValue
      architecturesValue="$(get_library_properties_field_value "$libraryProperties" 'architectures')"
      # Check for empty architectures value
      if [[ "$architecturesValue" == "" ]]; then
        echo "ERROR: ${normalizedLibraryPropertiesPath}/library.properties: Undefined architectures field. This causes the examples to be put under File > Examples > INCOMPATIBLE."
        exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_ARCHITECTURES_EMPTY_EXIT_STATUS)
      else
        # Check for invalid architectures
        local validArchitecturesRegex='^((\*)|(avr)|(sam)|(samd)|(stm32f4)|(nrf52)|(i586)|(i686)|(arc32)|(win10)|(esp8266)|(esp32)|(ameba)|(arm)|(efm32)|(FP51)|(iot2000)|(megaavr)|(msp430)|(navspark)|(nRF5)|(nRF51822)|(nRF52832)|(particle-photon)|(particle-electron)|(particle-core)|(pic)|(pic32)|(RFduino)|(Seeed_STM32F4)|(Simblee)|(solox)|(stm32)|(stm)|(STM32)|(STM32F1)|(STM32F3)|(STM32F4)|(STM32F2)|(STM32L1)|(STM32L4)|(teensy)|(x86))$'
        # Split string on ,
        IFS=','
        local validArchitectureFound=false
        # Disable globbing, otherwise it fails when one of the architecture values is *
        set -o noglob
        # Check for * architecture. If this is found then the other architecture values don't matter
        local wildcardArchitectureFound=false
        for rawArchitecture in $architecturesValue; do
          # The Arduino IDE ignores leading or trailing whitespace on architectures
          # Strip leading whitespace
          local architecture="${rawArchitecture#"${rawArchitecture%%[![:space:]]*}"}"
          # Strip trailing whitespace
          architecture="${architecture%"${architecture##*[![:space:]]}"}"

          if [[ "$architecture" == "*" ]]; then
            wildcardArchitectureFound=true
            validArchitectureFound=true
            break
          fi
        done
        if [[ "$wildcardArchitectureFound" == false ]]; then
          for rawArchitecture in $architecturesValue; do
            # The Arduino IDE ignores leading or trailing whitespace on architectures
            # Strip leading whitespace
            local architecture="${rawArchitecture#"${rawArchitecture%%[![:space:]]*}"}"
            # Strip trailing whitespace
            architecture="${architecture%"${architecture##*[![:space:]]}"}"

            if [[ "$architecture" =~ $validArchitecturesRegex ]]; then
              validArchitectureFound=true
            else
              local aliasCheckPassed=true
              # If an architecture alias is used then the correct architecture must also be present
              check_architecture_alias "$architecture" '^((Avr)|(AVR)|([aA][tT][mM][eE][lL].?[aA][vV][rR]))$' "$architecturesValue" 'avr' "$normalizedLibraryPropertiesPath"
              if [[ "$?" == "$ARDUINO_CI_SCRIPT_FAILURE_EXIT_STATUS" ]]; then
                aliasCheckPassed=false
                exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_INVALID_ARCHITECTURE_EXIT_STATUS)
              fi
              check_architecture_alias "$architecture" '^((Sam)|(SAM))$' "$architecturesValue" 'sam' "$normalizedLibraryPropertiesPath"
              if [[ "$?" == "$ARDUINO_CI_SCRIPT_FAILURE_EXIT_STATUS" ]]; then
                aliasCheckPassed=false
                exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_INVALID_ARCHITECTURE_EXIT_STATUS)
              fi
              check_architecture_alias "$architecture" '^((Samd)|(SAMD)|(SamD)|((samD)|([aA][tT][mM][eE][lL].?[sS][aA][mM]))$' "$architecturesValue" 'samd' "$normalizedLibraryPropertiesPath"
              if [[ "$?" == "$ARDUINO_CI_SCRIPT_FAILURE_EXIT_STATUS" ]]; then
                aliasCheckPassed=false
                exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_INVALID_ARCHITECTURE_EXIT_STATUS)
              fi
              check_architecture_alias "$architecture" '^((Arc32)|(ARC32)|([aA][rR][cC].32)|([iI][nN][tT][eE][lL].?[aA][rR][cC]32))$' "$architecturesValue" 'arc32' "$normalizedLibraryPropertiesPath"
              if [[ "$?" == "$ARDUINO_CI_SCRIPT_FAILURE_EXIT_STATUS" ]]; then
                aliasCheckPassed=false
                exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_INVALID_ARCHITECTURE_EXIT_STATUS)
              fi
              check_architecture_alias "$architecture" '^((Esp8266)|(ESP8266)|([eE][sS][pP].8266)|(8266)|([eE][sS][pP])|([eE][sS][pP][rR][eE][sS][sS][iI][fF].?(8266)?))$' "$architecturesValue" 'esp8266' "$normalizedLibraryPropertiesPath"
              if [[ "$?" == "$ARDUINO_CI_SCRIPT_FAILURE_EXIT_STATUS" ]]; then
                aliasCheckPassed=false
                exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_INVALID_ARCHITECTURE_EXIT_STATUS)
              fi
              check_architecture_alias "$architecture" '^((Esp32)|(ESP32)|([eE][sS][pP].32)|(arduino-esp32)|([eE][sS][pP][rR][eE][sS][sS][iI][fF].?32))$' "$architecturesValue" 'esp32' "$normalizedLibraryPropertiesPath"
              if [[ "$?" == "$ARDUINO_CI_SCRIPT_FAILURE_EXIT_STATUS" ]]; then
                aliasCheckPassed=false
                exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_INVALID_ARCHITECTURE_EXIT_STATUS)
              fi
              check_architecture_alias "$architecture" '^((Teensy)|(TEENSY))$' "$architecturesValue" 'teensy' "$normalizedLibraryPropertiesPath"
              if [[ "$?" == "$ARDUINO_CI_SCRIPT_FAILURE_EXIT_STATUS" ]]; then
                aliasCheckPassed=false
                exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_INVALID_ARCHITECTURE_EXIT_STATUS)
              fi

              if [[ "$aliasCheckPassed" == true ]]; then
                echo "WARNING: ${normalizedLibraryPropertiesPath}/library.properties: architectures field contains an unknown architecture: ${architecture}. Note: architecture values are case-sensitive."
              fi
            fi
          done
        fi
        # Re-enable globbing
        set +o noglob
        # Set IFS back to default
        unset IFS

        # At least one known architecture must be present
        if [[ "$validArchitectureFound" == false ]]; then
          echo "ERROR: ${normalizedLibraryPropertiesPath}/library.properties: architectures field (${architecturesValue}) doesn't contain any known architecture values."
          exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_INVALID_ARCHITECTURE_EXIT_STATUS)
        fi
      fi
    fi

    # Check for invalid lines (anything other than property, comment, or blank line)
    if grep --quiet --invert-match --extended-regexp --regexp='=' --regexp='^[[:space:]]*(#|$)' <<<"$libraryProperties"; then
      echo "ERROR: ${normalizedLibraryPropertiesPath}/library.properties: Invalid line found. Installation of a library with invalid line will cause all compilations to fail. library.properties must only consist of property definitions, blank lines, and comments (#)."
      exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_INVALID_LINE_EXIT_STATUS)
    fi

    # Check for incorrect includes field name case
    if ! check_field_name_case "$libraryProperties" 'includes' "$normalizedLibraryPropertiesPath"; then
      exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_INCLUDES_MISSPELLED_EXIT_STATUS)
    fi

    # Check for misspelled includes field name
    if grep --quiet --ignore-case --regexp='^[[:space:]]*include[[:space:]]*=' <<<"$libraryProperties"; then
      echo "ERROR: ${normalizedLibraryPropertiesPath}/library.properties: Misspelled includes field name. It must be spelled exactly \"includes\". See https://github.com/arduino/Arduino/wiki/Arduino-IDE-1.5:-Library-specification#libraryproperties-file-format"
      exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_INCLUDES_MISSPELLED_EXIT_STATUS)
    fi

    # Check for empty includes value
    if grep --quiet --regexp='^[[:space:]]*includes[[:space:]]*=[[:space:]]*$' <<<"$libraryProperties"; then
      echo "ERROR: ${normalizedLibraryPropertiesPath}/library.properties: Undefined includes field. Either define the field or remove it. See https://github.com/arduino/Arduino/wiki/Arduino-IDE-1.5:-Library-specification#libraryproperties-file-format"
      exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_EMPTY_INCLUDES_EXIT_STATUS)
    fi

    # Check for incorrect dot_a_linkage field name case
    if ! check_field_name_case "$libraryProperties" 'dot_a_linkage' "$normalizedLibraryPropertiesPath"; then
      exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_DOT_A_LINKAGE_MISSPELLED_EXIT_STATUS)
    fi

    # Check for misspelled dot_a_linkage field name
    if grep --quiet --ignore-case --extended-regexp --regexp='^[[:space:]]*((dot_a_linkages)|(dot-?a-?linkages?))[[:space:]]*=' <<<"$libraryProperties"; then
      echo "ERROR: ${normalizedLibraryPropertiesPath}/library.properties: Misspelled dot_a_linkage field name. It must be spelled exactly \"dot_a_linkage\". See https://github.com/arduino/Arduino/wiki/Arduino-IDE-1.5:-Library-specification#libraryproperties-file-format"
      exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_DOT_A_LINKAGE_MISSPELLED_EXIT_STATUS)
    fi

    # Check for incorrect precompiled field name case
    if ! check_field_name_case "$libraryProperties" 'precompiled' "$normalizedLibraryPropertiesPath"; then
      exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_PRECOMPILED_MISSPELLED_EXIT_STATUS)
    fi

    # Check for misspelled precompiled field name
    if grep --quiet --ignore-case --extended-regexp --regexp='^[[:space:]]*((precompile)|(pre[-_]compiled?))[[:space:]]*=' <<<"$libraryProperties"; then
      echo "ERROR: ${normalizedLibraryPropertiesPath}/library.properties: Misspelled precompiled field name. It must be spelled exactly \"precompiled\". See https://github.com/arduino/Arduino/wiki/Arduino-IDE-1.5:-Library-specification#libraryproperties-file-format"
      exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_PRECOMPILED_MISSPELLED_EXIT_STATUS)
    fi

    # Check for incorrect ldflags field name case
    if ! check_field_name_case "$libraryProperties" 'ldflags' "$normalizedLibraryPropertiesPath"; then
      exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_LDFLAGS_MISSPELLED_EXIT_STATUS)
    fi

    # Check for misspelled ldflags field name
    if grep --quiet --ignore-case --extended-regexp --regexp='^[[:space:]]*((ldflag)|(ld[-_]flags?))[[:space:]]*=' <<<"$libraryProperties"; then
      echo "ERROR: ${normalizedLibraryPropertiesPath}/library.properties: Misspelled ldflags field name. It must be spelled exactly \"ldflags\". See https://github.com/arduino/Arduino/wiki/Arduino-IDE-1.5:-Library-specification#libraryproperties-file-format"
      exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_PROPERTIES_LDFLAGS_MISSPELLED_EXIT_STATUS)
    fi

  done <<<"$(find "$normalizedLibraryPropertiesSearchPath" -maxdepth "$maximumSearchDepth" -type d)"

  return "$exitStatus"
}

# https://github.com/arduino/Arduino/wiki/Arduino-IDE-1.5:-Library-specification#keywords
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=1
readonly ARDUINO_CI_SCRIPT_CHECK_KEYWORDS_TXT_INCONSEQUENTIAL_MULTIPLE_TABS_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_KEYWORDS_TXT_INVALID_LINE_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_KEYWORDS_TXT_INCONSEQUENTIAL_LEADING_SPACE_ON_KEYWORD_TOKENTYPE_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_KEYWORDS_TXT_MULTIPLE_TABS_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_KEYWORDS_TXT_LEADING_SPACE_ON_KEYWORD_TOKENTYPE_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_KEYWORDS_TXT_FOLDER_DOESNT_EXIST_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_KEYWORDS_TXT_MISSPELLED_FILENAME_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_KEYWORDS_TXT_INCORRECT_FILENAME_CASE_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_KEYWORDS_TXT_INVALID_FIELD_SEPARATOR_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_KEYWORDS_TXT_BOM_CORRUPTED_KEYWORD_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_KEYWORDS_TXT_INVALID_KEYWORD_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_KEYWORDS_TXT_INVALID_KEYWORD_TOKENTYPE_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_KEYWORDS_TXT_LEADING_SPACE_ON_RSYNTAXTEXTAREA_TOKENTYPE_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_KEYWORDS_TXT_INVALID_RSYNTAXTEXTAREA_TOKENTYPE_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_KEYWORDS_TXT_INVALID_REFERENCE_LINK_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
function check_keywords_txt() {
  local -r keywordsTxtSearchPath="$1"
  local maximumSearchDepth="$2"
  if [[ "$maximumSearchDepth" == "" ]]; then
    # Set default search depth
    maximumSearchDepth=0
  fi

  local exitStatus=$ARDUINO_CI_SCRIPT_SUCCESS_EXIT_STATUS

  # Replace backslashes with slashes
  local -r keywordsTxtSearchPathWithSlashes="${keywordsTxtSearchPath//\\//}"
  local -r keywordsTxtRegex='[kK][eE][yY][wW][oO][rR][dD][sS]\.[tT][xX][tT]$'
  if [[ ! -d "$keywordsTxtSearchPathWithSlashes" && "$keywordsTxtSearchPathWithSlashes" =~ $keywordsTxtRegex ]]; then
    # Path contains the filename but we only want the folder
    local -r keywordsTxtSearchPathWithoutFile="${keywordsTxtSearchPathWithSlashes::-12}"
  else
    local -r keywordsTxtSearchPathWithoutFile="$keywordsTxtSearchPathWithSlashes"
  fi
  # Remove trailing slash
  local -r normalizedKeywordsTxtSearchPath="${keywordsTxtSearchPathWithoutFile%/}"

  # Check whether folder exists
  if [[ ! -d "$normalizedKeywordsTxtSearchPath" ]]; then
    echo "ERROR: ${normalizedKeywordsTxtSearchPath}: Folder doesn't exist."
    return $ARDUINO_CI_SCRIPT_CHECK_KEYWORDS_TXT_FOLDER_DOESNT_EXIST_EXIT_STATUS
  fi

  while read -r normalizedKeywordsTxtPath; do
    # Check for misspelled keywords.txt filename
    if [[ "$(find "$normalizedKeywordsTxtPath" -type f -iname 'keyword.txt')" || "$(find "$normalizedKeywordsTxtPath" -type f -iregex '.*/keywords?\.text')" || "$(find "$normalizedKeywordsTxtPath" -type f -iregex '.*/keywords?\.txt\.txt')" ]]; then
      echo "ERROR: ${normalizedKeywordsTxtPath}: Incorrectly spelled keywords.txt file. It must be spelled exactly \"keywords.txt\"."
      exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_KEYWORDS_TXT_MISSPELLED_FILENAME_EXIT_STATUS)
    fi

    # Check for incorrect filename case
    local keywordsTxtFound=false
    while read -r foundKeywordsTxtPath; do
      # The while loop always runs once, even if no file was found
      if [[ "$foundKeywordsTxtPath" == "" ]]; then
        continue
      fi

      if [[ "${foundKeywordsTxtPath: -12}" == 'keywords.txt' ]]; then
        keywordsTxtFound=true
      else
        echo "ERROR: ${foundKeywordsTxtPath}: Incorrect filename case, which causes it to not be recognized on a filename case-sensitive OS such as Linux. It must be exactly \"keywords.txt\"."
        exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_KEYWORDS_TXT_INCORRECT_FILENAME_CASE_EXIT_STATUS)
      fi
    done <<<"$(find "$normalizedKeywordsTxtPath" -maxdepth 1 -type f -iname 'keywords.txt')"

    # Check whether the folder contains a keywords.txt file
    if [[ "$keywordsTxtFound" == false ]]; then
      # no point in doing any more checks on this folder
      continue
    fi

    # Read the keywords.txt file line by line
    # Split into lines by CR
    while IFS='' read -d $'\r' -r keywordsTxtCRline || [[ -n "$keywordsTxtCRline" ]]; do
      # Split into lines by LN
      while IFS='' read -r keywordsTxtLine || [[ -n "$keywordsTxtLine" ]]; do
        # Skip blank lines and comments
        local blankLineRegex='^[[:space:]]*$'
        local commentRegex='^[[:space:]]*#'
        if [[ "$keywordsTxtLine" =~ $blankLineRegex ]] || [[ "$keywordsTxtLine" =~ $commentRegex ]]; then
          continue
        fi

        # Skip BOM corrupted blank lines and comments
        if grep --quiet $'\xEF\xBB\xBF' <<<"$keywordsTxtLine"; then
          local BOMcorruptedCommentRegex='^.[[:space:]]*#'
          local BOMcorruptedBlankLineRegex='^.[[:space:]]*$'
          if [[ "$keywordsTxtLine" =~ $BOMcorruptedCommentRegex ]] || [[ "$keywordsTxtLine" =~ $BOMcorruptedBlankLineRegex ]]; then
            echo "WARNING: ${normalizedKeywordsTxtPath}/keywords.txt: BOM found. In this case it does not cause an issue but it's recommended to use UTF-8 encoding for keywords.txt."
            continue
          fi
        fi

        local spacesSeparatorRegex='^[[:space:]]*[^[:space:]]+ +[^[:space:]]+'
        # Check for invalid separator
        if [[ "$keywordsTxtLine" =~ $spacesSeparatorRegex ]]; then
          echo "ERROR: ${normalizedKeywordsTxtPath}/keywords.txt: Space(s) used as a field separator. Fields must be separated by a single true tab."
          echo -e "\t$keywordsTxtLine"
          exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_KEYWORDS_TXT_INVALID_FIELD_SEPARATOR_EXIT_STATUS)
          # The rest of the checks will be borked by messed up field separators so continue to the next line
          continue
        fi

        # Check for multiple tabs used as separator where this causes unintended results
        local consequentialMultipleSeparatorRegex='^[[:space:]]*[^[:space:]]+[[:space:]]*'$'\t''+[[:space:]]*'$'\t''+[[:space:]]*((KEYWORD1)|(LITERAL1))'
        if [[ "$keywordsTxtLine" =~ $consequentialMultipleSeparatorRegex ]]; then
          echo "ERROR: ${normalizedKeywordsTxtPath}/keywords.txt: Multiple tabs used as field separator. It must be a single tab. This causes the default keyword highlighting (as used by KEYWORD2, KEYWORD3, LITERAL2) to be used rather than the intended highlighting."
          echo -e "\t$keywordsTxtLine"
          exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_KEYWORDS_TXT_MULTIPLE_TABS_EXIT_STATUS)
          # The rest of the checks will be borked by messed up field separators so continue to the next line
          continue
        fi

        # Check for multiple tabs used as separator where this causes no unintended results
        local inconsequentialMultipleSeparatorRegex='^[[:space:]]*[^[:space:]]+[[:space:]]*'$'\t''+[[:space:]]*'$'\t''+[[:space:]]*((KEYWORD2)|(KEYWORD3)|(LITERAL2))'
        if [[ "$keywordsTxtLine" =~ $inconsequentialMultipleSeparatorRegex ]]; then
          echo "ERROR: ${normalizedKeywordsTxtPath}/keywords.txt: Multiple tabs used as field separator. It must be a single tab. This causes the default keyword highlighting (as used by KEYWORD2, KEYWORD3, LITERAL2). In this case that doesn't cause the keywords to be colored other than intended but it's recommended to fully comply with the Arduino library specification."
          echo -e "\t$keywordsTxtLine"
          exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_KEYWORDS_TXT_INCONSEQUENTIAL_MULTIPLE_TABS_EXIT_STATUS)
          # The rest of the checks will be borked by messed up field separators so continue to the next line
          continue
        fi

        # Check for invalid line
        local invalidLineRegex='^[[:space:]]*[^[:space:]]+[[:space:]]*$'
        if [[ "$keywordsTxtLine" =~ $invalidLineRegex ]]; then
          echo "ERROR: ${normalizedKeywordsTxtPath}/keywords.txt: Invalid line. If this was intended as a comment, it should use the correct # syntax."
          echo -e "\t$keywordsTxtLine"
          exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_KEYWORDS_TXT_INVALID_LINE_EXIT_STATUS)
          # The rest of the checks are pointless so continue to the next line of keywords.txt
          continue
        fi

        # Get the field values
        # Use a unique, non-whitespace field separator character
        fieldSeparator=$'\a'
        IFS=$fieldSeparator
        # Strip leading whitespace. This is ignored by the Arduino IDE (even tabs)
        local keywordsTxtLineFrontStripped="${keywordsTxtLine#"${keywordsTxtLine%%[![:space:]]*}"}"
        # Change tabs to the field separator character for line splitting
        local keywordsTxtLineSwappedTabs=(${keywordsTxtLineFrontStripped//$'\t'/$fieldSeparator})

        # KEYWORD is the 1st field
        local keywordRaw=${keywordsTxtLineSwappedTabs[0]}
        # Strip trailing spaces
        local keyword="${keywordRaw%"${keywordRaw##*[! ]}"}"

        # KEYWORD_TOKENTYPE is the 2nd field
        local keywordTokentypeRaw=${keywordsTxtLineSwappedTabs[1]}
        # The Arduino IDE strips trailing spaces from KEYWORD_TOKENTYPE
        # Strip trailing spaces
        local keywordTokentype="${keywordTokentypeRaw%"${keywordTokentypeRaw##*[! ]}"}"

        # REFERENCE_LINK is the 3rd field
        local referenceLinkRaw=${keywordsTxtLineSwappedTabs[2]}
        # The Arduino IDE strips leading and trailing whitespace from REFERENCE_LINK
        # Strip leading spaces
        local referenceLinkFrontStripped="${referenceLinkRaw#"${referenceLinkRaw%%[! ]*}"}"
        # Strip trailing spaces
        local referenceLink="${referenceLinkFrontStripped%"${referenceLinkFrontStripped##*[! ]}"}"

        # RSYNTAXTEXTAREA_TOKENTYPE is the 4th field
        local rsyntaxtextareaTokentypeRaw=${keywordsTxtLineSwappedTabs[3]}
        # The Arduino IDE strips trailing spaces from RSYNTAXTEXTAREA_TOKENTYPE
        # Strip trailing spaces
        local rsyntaxtextareaTokentype="${rsyntaxtextareaTokentypeRaw%"${rsyntaxtextareaTokentypeRaw##*[! ]}"}"

        # Reset IFS to default
        unset IFS

        allowedKeywordCharactersRegex='^[a-zA-Z0-9_]+$'
        # Check for corruption of KEYWORD field caused by UTF-8 BOM file encoding
        if grep --quiet $'\xEF\xBB\xBF' <<<"$keyword"; then
          echo "ERROR: ${normalizedKeywordsTxtPath}/keywords.txt: UTF-8 BOM file encoding has corrupted the first keyword definition. Please change the file encoding to standard UTF-8."
          exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_KEYWORDS_TXT_BOM_CORRUPTED_KEYWORD_EXIT_STATUS)
        elif ! [[ "$keyword" =~ $allowedKeywordCharactersRegex ]]; then
          # Check for invalid characters in KEYWORD
          # The Arduino IDE does recognize keywords that start with a number, even though these are not valid identifiers.
          echo "ERROR: ${normalizedKeywordsTxtPath}/keywords.txt: Keyword: $keyword contains invalid character(s), which causes it to not be recognized by the Arduino IDE. Keywords may only contain the characters a-z, A-Z, 0-9, and _."
          echo -e "\t$keywordsTxtLine"
          exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_KEYWORDS_TXT_INVALID_KEYWORD_EXIT_STATUS)
        fi

        # Check for invalid KEYWORD_TOKENTYPE
        local validKeywordTokentypeRegex='^((KEYWORD1)|(KEYWORD2)|(KEYWORD3)|(LITERAL1)|(LITERAL2))$'
        local validRsyntaxtextareaTokentypeRegex='^((RESERVED_WORD)|(RESERVED_WORD_2)|(DATA_TYPE)|(PREPROCESSOR)|(LITERAL_BOOLEAN))$'
        if ! [[ "$keywordTokentype" =~ $validKeywordTokentypeRegex ]]; then
          # Check if it's invalid only because of leading space
          local keywordTokentypeWithoutLeadingSpace="${keywordTokentype#"${keywordTokentype%%[![:space:]]*}"}"
          if [[ "$keywordTokentypeWithoutLeadingSpace" =~ $validKeywordTokentypeRegex ]]; then
            # Check if the issue doesn't cause any change from the intended highlighting
            local inconsequentialTokentypeRegex='((KEYWORD2)|(KEYWORD3)|(LITERAL2))'
            if [[ "$keywordTokentypeWithoutLeadingSpace" =~ $inconsequentialTokentypeRegex ]]; then
              echo "ERROR: ${normalizedKeywordsTxtPath}/keywords.txt: Leading space on the KEYWORD_TOKENTYPE field causes it to not be recognized, so the default keyword highlighting is used. In this case that doesn't cause the keywords to be colored other than intended but it's recommended to fully comply with the Arduino library specification."
              echo -e "\t$keywordsTxtLine"
              exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_KEYWORDS_TXT_INCONSEQUENTIAL_LEADING_SPACE_ON_KEYWORD_TOKENTYPE_EXIT_STATUS)
            else
              echo "ERROR: ${normalizedKeywordsTxtPath}/keywords.txt: Leading space on the KEYWORD_TOKENTYPE field causes it to not be recognized, so the default keyword highlighting is used."
              echo -e "\t$keywordsTxtLine"
              exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_KEYWORDS_TXT_LEADING_SPACE_ON_KEYWORD_TOKENTYPE_EXIT_STATUS)
            fi
          elif ! [[ "$keywordTokentypeWithoutLeadingSpace" == "" && "$rsyntaxtextareaTokentype" =~ $validRsyntaxtextareaTokentypeRegex ]]; then
            # It's reasonable to leave KEYWORD_TOKENTYPE blank if RSYNTAXTEXTAREA_TOKENTYPE is defined and valid. This will not be compatible with 1.6.4 and older but that's really no big deal.
            echo "ERROR: ${normalizedKeywordsTxtPath}/keywords.txt: Invalid KEYWORD_TOKENTYPE: $keywordTokentype causes the default keyword highlighting to be used. See: https://github.com/arduino/Arduino/wiki/Arduino-IDE-1.5:-Library-specification#keyword_tokentype"
            echo -e "\t$keywordsTxtLine"
            exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_KEYWORDS_TXT_INVALID_KEYWORD_TOKENTYPE_EXIT_STATUS)
          fi
        fi

        # Check for invalid REFERENCE_LINK
        if [[ "$referenceLink" != "" ]]; then
          # The Arduino IDE must be installed to check if the reference page exists
          if [[ "$NEWEST_INSTALLED_IDE_VERSION" == "" ]]; then
            echo "WARNING: Arduino IDE is not installed so unable to check for invalid reference links. Please call install_ide before running check_keywords_txt."
          else
            install_ide_version "$NEWEST_INSTALLED_IDE_VERSION"
            if [[ ! $(find "${ARDUINO_CI_SCRIPT_APPLICATION_FOLDER}/${ARDUINO_CI_SCRIPT_IDE_INSTALLATION_FOLDER}/reference/www.arduino.cc/en/Reference/" -type f -name "${referenceLink}.html") ]]; then
              echo "ERROR: ${normalizedKeywordsTxtPath}/keywords.txt: REFERENCE_LINK value: $referenceLink is not a valid Arduino Language Reference page. See: https://github.com/arduino/Arduino/wiki/Arduino-IDE-1.5:-Library-specification#reference_link"
              echo -e "\t$keywordsTxtLine"
              exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_KEYWORDS_TXT_INVALID_REFERENCE_LINK_EXIT_STATUS)
            fi
          fi
        fi

        # Check for invalid RSYNTAXTEXTAREA_TOKENTYPE
        if [[ "$rsyntaxtextareaTokentype" != "" ]]; then
          if ! [[ "$rsyntaxtextareaTokentype" =~ $validRsyntaxtextareaTokentypeRegex ]]; then
            # Check if it's invalid only because of leading space
            local rsyntaxtextareaTokentypeWithoutLeadingSpace="${rsyntaxtextareaTokentype#"${rsyntaxtextareaTokentype%%[![:space:]]*}"}"
            if [[ "$rsyntaxtextareaTokentypeWithoutLeadingSpace" =~ $validRsyntaxtextareaTokentypeRegex ]]; then
              echo "ERROR: ${normalizedKeywordsTxtPath}/keywords.txt: Leading space on the RSYNTAXTEXTAREA_TOKENTYPE field causes it to not be recognized, so the default keyword highlighting is used."
              echo -e "\t$keywordsTxtLine"
              exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_KEYWORDS_TXT_LEADING_SPACE_ON_RSYNTAXTEXTAREA_TOKENTYPE_EXIT_STATUS)
            else
              echo "ERROR: ${normalizedKeywordsTxtPath}/keywords.txt: Invalid RSYNTAXTEXTAREA_TOKENTYPE: $rsyntaxtextareaTokentype causes the default keyword highlighting to be used. See: https://github.com/arduino/Arduino/wiki/Arduino-IDE-1.5:-Library-specification#rsyntaxtextarea_tokentype"
              echo -e "\t$keywordsTxtLine"
              exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_KEYWORDS_TXT_INVALID_RSYNTAXTEXTAREA_TOKENTYPE_EXIT_STATUS)
            fi
          fi
        fi
      done <<<"$keywordsTxtCRline"
    done <"${normalizedKeywordsTxtPath}/keywords.txt"

  done <<<"$(find "$normalizedKeywordsTxtSearchPath" -maxdepth "$maximumSearchDepth" -type d)"

  return "$exitStatus"
}

# https://github.com/arduino/Arduino/wiki/Library-Manager-FAQ#how-is-the-library-list-generated
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=1
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_MANAGER_COMPLIANCE_FOLDER_DOESNT_EXIST_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_MANAGER_COMPLIANCE_EXE_FOUND_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_MANAGER_COMPLIANCE_DOT_DEVELOPMENT_FOUND_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_MANAGER_COMPLIANCE_SYMLINK_FOUND_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_MANAGER_COMPLIANCE_CHECK_FOLDER_NAME_OFFSET=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_MANAGER_COMPLIANCE_NAME_HAS_INVALID_FIRST_CHARACTER_EXIT_STATUS=$((ARDUINO_CI_SCRIPT_CHECK_LIBRARY_MANAGER_COMPLIANCE_CHECK_FOLDER_NAME_OFFSET + ARDUINO_CI_SCRIPT_CHECK_FOLDER_NAME_INVALID_FIRST_CHARACTER_EXIT_STATUS))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_MANAGER_COMPLIANCE_NAME_HAS_INVALID_CHARACTER_EXIT_STATUS=$((ARDUINO_CI_SCRIPT_CHECK_LIBRARY_MANAGER_COMPLIANCE_CHECK_FOLDER_NAME_OFFSET + ARDUINO_CI_SCRIPT_CHECK_FOLDER_NAME_INVALID_CHARACTER_EXIT_STATUS))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_MANAGER_COMPLIANCE_NAME_TOO_LONG_EXIT_STATUS=$((ARDUINO_CI_SCRIPT_CHECK_LIBRARY_MANAGER_COMPLIANCE_CHECK_FOLDER_NAME_OFFSET + ARDUINO_CI_SCRIPT_CHECK_FOLDER_NAME_TOO_LONG_EXIT_STATUS))
ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER=$((ARDUINO_CI_SCRIPT_CHECK_LIBRARY_MANAGER_COMPLIANCE_NAME_TOO_LONG_EXIT_STATUS + 1))
readonly ARDUINO_CI_SCRIPT_CHECK_LIBRARY_MANAGER_COMPLIANCE_BLANK_URL_EXIT_STATUS=$ARDUINO_CI_SCRIPT_EXIT_STATUS_COUNTER
function check_library_manager_compliance() {
  local -r libraryPath="$1"
  # Replace backslashes with slashes
  local -r libraryPathWithSlashes="${libraryPath//\\//}"
  # Remove trailing slash
  local -r normalizedLibraryPath="${libraryPathWithSlashes%/}"

  local exitStatus=$ARDUINO_CI_SCRIPT_SUCCESS_EXIT_STATUS

  # Check whether folder exists
  if [[ ! -d "$normalizedLibraryPath" ]]; then
    echo "ERROR: ${libraryPath}: Folder doesn't exist."
    return $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_MANAGER_COMPLIANCE_FOLDER_DOESNT_EXIST_EXIT_STATUS
  fi

  # Check for .exe files
  local -r exePath=$(find "$normalizedLibraryPath" -type f -name '*.exe')
  if [[ "$exePath" != "" ]]; then
    echo "ERROR: ${exePath}: .exe file found."
    exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_MANAGER_COMPLIANCE_EXE_FOUND_EXIT_STATUS)
  fi

  # Check for .development file
  local -r dotDevelopmentPath=$(find "$normalizedLibraryPath" -maxdepth 1 -type f -name '.development')
  if [[ "$dotDevelopmentPath" != "" ]]; then
    echo "ERROR: ${dotDevelopmentPath}: .development file found."
    exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_MANAGER_COMPLIANCE_DOT_DEVELOPMENT_FOUND_EXIT_STATUS)
  fi

  # Check for symlink
  local -r symlinkPath=$(find "$normalizedLibraryPath" -type l)
  if [[ "$symlinkPath" != "" ]]; then
    echo "ERROR: ${symlinkPath}: Symlink found."
    exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_MANAGER_COMPLIANCE_SYMLINK_FOUND_EXIT_STATUS)
  fi

  # Check for characters in the library.properties name value disallowed by the Library Manager indexer
  if [[ -f "$normalizedLibraryPath/library.properties" ]]; then
    # Get rid of the CRs
    local libraryProperties
    libraryProperties=$(tr "\r" "\n" <"$normalizedLibraryPath/library.properties")
    local nameValue
    nameValue="$(get_library_properties_field_value "$libraryProperties" 'name')"
    # Library Manager installs libraries to a folder that is the name field value with any spaces replaced with _
    local -r libraryManagerFolderName="${nameValue// /_}"
    check_folder_name "$libraryManagerFolderName"
    local -r checkFolderNameExitStatus=$?
    if [[ $checkFolderNameExitStatus -ne $ARDUINO_CI_SCRIPT_SUCCESS_EXIT_STATUS ]]; then
      echo "ERROR: ${normalizedLibraryPath}/library.properties: name value: $nameValue does not meet the requirements of the Arduino Library Manager indexer. See: https://github.com/arduino/Arduino/wiki/Arduino-IDE-1.5:-Library-specification#libraryproperties-file-format"
      exitStatus=$(set_exit_status "$exitStatus" $((ARDUINO_CI_SCRIPT_CHECK_LIBRARY_MANAGER_COMPLIANCE_CHECK_FOLDER_NAME_OFFSET + checkFolderNameExitStatus)))
    fi

    local urlValue
    urlValue="$(get_library_properties_field_value "$libraryProperties" 'url')"
    if [[ "$urlValue" == "" ]]; then
      echo "ERROR: ${normalizedLibraryPath}/library.properties: Blank url value: You must define a URL."
      exitStatus=$(set_exit_status "$exitStatus" $ARDUINO_CI_SCRIPT_CHECK_LIBRARY_MANAGER_COMPLIANCE_BLANK_URL_EXIT_STATUS)
    fi
  fi

  return "$exitStatus"
}

# Set default verbosity (must be called after the function definitions
set_script_verbosity 0

# Create the temporary folder
rm "$ARDUINO_CI_SCRIPT_TEMPORARY_FOLDER" --recursive --force
create_folder "$ARDUINO_CI_SCRIPT_TEMPORARY_FOLDER"

# Create the report folder
create_folder "$ARDUINO_CI_SCRIPT_REPORT_FOLDER"

# Add column names to report
echo "Build Timestamp (UTC)"$'\t'"Build"$'\t'"Job"$'\t'"Job URL"$'\t'"Build Trigger"$'\t'"Allow Job Failure"$'\t'"PR#"$'\t'"Branch"$'\t'"Commit"$'\t'"Commit Range"$'\t'"Commit Message"$'\t'"Sketch Filename"$'\t'"Board ID"$'\t'"IDE Version"$'\t'"Program Storage (bytes)"$'\t'"Dynamic Memory (bytes)"$'\t'"# Warnings"$'\t'"Allow Failure"$'\t'"Exit Status"$'\t'"# Board Issues"$'\t'"Board Issue"$'\t'"# Library Issues"$'\t'"Library Issue"$'\r' >"$ARDUINO_CI_SCRIPT_REPORT_FILE_PATH"
