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
  "strconv"
  "time"
  "github.com/wcharczuk/go-chart"
)

var (
  dbPathFlag = flag.String("db-path", "./temp.db", "Path to the database with temperature notes")
)

const (
  probesPerHour = 4
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
  
  th.selectStmt, err = th.db.Prepare("SELECT * FROM Temps ORDER BY time DESC LIMIT ?")
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

func (th *TempHandler) ServeJSON(rw http.ResponseWriter, request *http.Request) {
  log.Printf("Processing request from %v", request.RemoteAddr)
  
  daysParam := request.URL.Query().Get("days")
  daysNumber, err := strconv.Atoi(daysParam)
  if err != nil { daysNumber = 1 }

  tempsArr := th.queryMetrics(daysNumber)
  temps, _ := json.Marshal(tempsArr)
  byteArray := bytes.NewBuffer(temps).Bytes()

  log.Printf("%v bytes to send", len(byteArray))

  rw.Write(byteArray)
}

func (th *TempHandler) ServePNG(rw http.ResponseWriter, request *http.Request) {
  log.Printf("Processing request from %v", request.RemoteAddr)
  
  daysParam := request.URL.Query().Get("days")
  daysNumber, err := strconv.Atoi(daysParam)
  if err != nil { daysNumber = 1 }

  tempsArr := th.queryMetrics(daysNumber)

  var x, y []float64

  for _, t := range tempsArr {
    x = append(x, t.Time)
    y = append(y, t.Temperature)
  }

  graph := chart.Chart{
    Series: []chart.Series{
        chart.ContinuousSeries{
            XValues: x,
            YValues: y,
        },
    },
  }

  rw.Header().Set("Content-Type", "image/png")
  graph.Render(chart.PNG, rw)
} 

func (th *TempHandler) queryMetrics(daysNumber int) []TempData {
  var timestamp, maxTimestamp int64
  var sensorID int
  var temperature float32

  log.Printf("%v last days to query", daysNumber)

  rows, err := th.selectStmt.Query((daysNumber + 1) * 24 * probesPerHour)
  if err != nil { return []TempData{} }

  var tempsArr []TempData
  
  for rows.Next() {
    err = rows.Scan(&timestamp, &sensorID, &temperature)
    if err != nil { continue }
  
    tempsArr = append(tempsArr, TempData{timestamp, sensorID, temperature})
    if (timestamp > maxTimestamp) { maxTimestamp = timestamp }
  }
  
  rows.Close()

  log.Printf("Last date found is %v", time.Unix(maxTimestamp, 0))
  firstTime := time.Unix(maxTimestamp, 0).Add(-time.Hour*24*time.Duration(daysNumber)).Unix()

  j := 0
  for i, td := range tempsArr {
    j = i
    if td.Time < firstTime { break }
  }

  log.Printf("%v objects to return out of %v", j, len(tempsArr))    
  return tempsArr[:j]
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
  http.HandleFunc("/temps", func(w http.ResponseWriter, r *http.Request) {
    th.ServeJSON(w, r)
  })

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
