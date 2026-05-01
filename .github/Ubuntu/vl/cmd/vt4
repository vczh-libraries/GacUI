#!/bin/bash

function Help {
    echo "Usage:"
    echo "--preprocess <t4-template-file>"
    echo "    Preprocess the template file and print to standard output."
    echo "--evaluate <t4-template-file>"
    echo "    Evaluate the preprocessed template file and print to standard output."
}

function Error {
    (>&2 echo "Error(Row:${ROW_NUMBER}, Col:${COLUMN_NUMBER}): $1")
    (>&2 echo "    In: ${LINE}")
    exit 1
}

function Preprocess {
    PATTERN='^\s*<#@\s*include\s*"([^"]*)"\s*#>\s*$'
    cat $1 | while read -r LINE; do
        INCLUDE_STAT=`echo "${LINE}" | grep -E ${PATTERN}`
        if [[ "$INCLUDE_STAT" == "" ]]; then
            echo "${LINE}"
        else
            INCLUDE_FILE=`echo "${LINE}" | sed -r -e 's%'${PATTERN}'%\1%g'`
            INCLUDE_DIR=`dirname ${INCLUDE_FILE}`
            INCLUDE_NAME=`basename ${INCLUDE_FILE}`

            eval INCLUDE_DIR=$INCLUDE_DIR
            pushd ${INCLUDE_DIR} > /dev/null
            vt4 --preprocess "${INCLUDE_NAME}"
            popd > /dev/null
        fi
    done
}

function Evaluate {
    STATE_TEXT=1
    STATE_CODE=2
    STATE_EXPR=3

    STATE=$STATE_TEXT
    ROW_NUMBER=1

    echo "#!/bin/bash"
    echo ""

    cat $1 | while read -r LINE; do
        COLUMN_NUMBER=0

        PARTS=`echo $LINE | sed -r -e 's%(<#=\s*|<#\s*|\s*#>)%\n\1\n%g'`
        PARTS=`echo "$PARTS" | sed -r -e 's%^<#=\s*$%<#=%g;s%^<#\s*$%<#%g;s%^\s*#>$%#>%g'`
        PURE_COMMAND=1

        while read -r PART; do
            case "$PART" in
                "")
                ;;

                "<#")
                if [ $STATE == $STATE_TEXT ]; then
                    STATE=$STATE_CODE
                else
                    Error "\"<#\" and \"<#=\" cannot be embedded in each other."
                fi
                ;;

                "<#=")
                if [ $STATE == $STATE_TEXT ]; then
                    STATE=$STATE_EXPR
                else
                    Error "\"<#\" and \"<#=\" cannot be embedded in each other."
                fi
                ;;

                "#>")
                if [ $STATE == $STATE_TEXT ]; then
                    Error "Wrong place for \"#>\"."
                else
                    STATE=$STATE_TEXT
                fi
                ;;

                *)
                if [ $STATE == $STATE_CODE ]; then
                    echo "${PART}"
                elif [ $STATE == $STATE_EXPR ]; then
                    PURE_COMMAND=0
                    echo "echo -n ${PART}"
                else
                    PURE_COMMAND=0
                    echo "echo -n '${PART}'"
                fi
                ;;
            esac

            COLUMN_NUMBER=`echo "$(($COLUMN_NUMBER+${#PARTS}))"`
        done <<< "${PARTS}"

        if ( [ "$LINE" == "" ] && [ $STATE == $STATE_TEXT ] ) || [ $PURE_COMMAND == 0 ]; then
            echo 'echo ""'
        fi
        ROW_NUMBER=`echo "$(($ROW_NUMBER+1))"`
    done
}

IFS=
case $1 in
    --help)
    Help
    ;;

    --preprocess)
    Preprocess $2
    ;;

    --evaluate)
    Evaluate $2
    ;;

    *)
    echo "Use --help for more information."
    ;;
esac
