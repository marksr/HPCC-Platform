import { createTheme } from "@material-ui/core/styles";

export const theme = createTheme({
    typography: {
        fontSize: 15
    },
    palette: {
      primary: {
          main: "#1a9bd7",
          light: "#66ccff",
          dark: "#006da5",
          contrastText: "#f5f5f5"
      },
      secondary: {
          main: "#455a64",
          light: "#718792",
          dark: "#1c313a",
          contrastText: "#fff"
      }
    },
    overrides: {  // lets make everything look the same. if we want to overwrite use the useStyles hook inside the component
        MuiStepIcon: {
            root: {
                color: "rgba(0, 0, 0, 0.38)"
            },
            completed: {
                color: "#1a9bd7"
            }
        },
        MuiTableContainer: {
            root: {
                margin: "20px 0px"
            }
        },
        MuiButton: {
            root: {
                "color": "white",
                "&:hover": {
                    background: "primary",
                    color: "white",
                    boxShadow: "none"
                }
            }
        },
        MuiTableCell: {
            head: {
               fontWeight: 600,
               width: "50%"
            }
        }
    }
});
