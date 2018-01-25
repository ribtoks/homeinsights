package main

import (
  "database/sql"
  _ "github.com/mattn/go-sqlite3"
  "encoding/json"
  "net/http"
  "bytes"
//  "time"
  "flag"
  "log"
  "os"
  "fmt"
)

var (
  dbPathFlag = flag.String("db-path", "./temp.db", "Path to the database with temperature notes")
)

type TempData struct {
  Time int64 `json:"time"`
  SensorID int `json:"sensorID"`
  Temperature float32 `json:"temperature"`
}

type TempHandler struct {
  db *sql.DB
  selectStmt *sql.Stmt
}

func (th *TempHandler) initDatabase() error {
  var err error
  th.db, err = sql.Open("sqlite3", *dbPathFlag)
  if err != nil {
    return err
  }
  
  log.Printf("Database %v opened", *dbPathFlag)
  
  th.selectStmt, err = th.db.Prepare("SELECT * FROM Temps ORDER BY time DESC LIMIT 1000")
  if err != nil {
    return err
  }

  log.Println("Statement prepared")
  
  return nil
}

func (th *TempHandler) finalizeDatabase() error {
  th.db.Close()  
  log.Println("Database closed")  
  return nil
}

func (th *TempHandler) ServeHTTP(rw http.ResponseWriter, request *http.Request) {
  var timestamp int64
  var sensorID int
  var temperature float32
  
  log.Printf("Processing request from %v", request.RemoteAddr)
  
  rows, err := th.selectStmt.Query()
  if err != nil { return }

  var tempsArr []TempData
  
  for rows.Next() {
    err = rows.Scan(&timestamp, &sensorID, &temperature)
    if err != nil { continue }
  
    tempsArr = append(tempsArr, TempData{timestamp, sensorID, temperature})
  }

  log.Printf("%v objects to return", len(tempsArr))

  temps, _ := json.Marshal(tempsArr)
  byteArray := bytes.NewBuffer(temps).Bytes()

  log.Printf("%v bytes to send", len(byteArray))

  rw.Write(byteArray)
  
  rows.Close()
}

func main() {
  flag.Parse()

  logfile, err := setupLogging()
  if err != nil {
    defer logfile.Close()
  }
  
  handler := &TempHandler{}
  err = handler.initDatabase()
  if err != nil {
    return
  }

  http.Handle("/", http.FileServer(http.Dir("./www")))
  http.Handle("/temps", handler)

  err = http.ListenAndServe(":8080", nil)

  handler.finalizeDatabase();
}

func setupLogging() (f *os.File, err error) {
  f, err = os.OpenFile("webserver.log", os.O_RDWR | os.O_CREATE | os.O_APPEND, 0666)
  if err != nil {
    fmt.Println("error opening logfile")
    return nil, err
  }

  log.SetOutput(f)

  log.Println("------------------------------")
  log.Println("log started")

  return f, err
}
