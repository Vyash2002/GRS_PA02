#!/bin/bash
export LC_ALL=C

# --- CONFIGURATION ---
PORT=9090
DURATION=5
RESULTS="MT25092_results.csv"

MSG_SIZES=(64 128 256 512)
THREADS=(1 2 4 8)

# Header: Changed l1_cache_misses -> page_faults
echo "impl,msg_size,threads,throughput_gbps,latency_us,cpu_cycles,page_faults,llc_cache_misses,context_switches" > "$RESULTS"

# --- HELPER FUNCTIONS ---

find_bin () {
  ls -1 *"$1"* 2>/dev/null | head -n 1
}

get_perf () {
  # Parses perf CSV output. Returns 0 if <not supported>
  local VAL=$(grep -i "$1" "$2" | head -n 1 | awk -F, '{print $1}')
  if [[ -z "$VAL" || "$VAL" == *"<"* ]]; then echo "0"; else echo "$VAL"; fi
}

cleanup() {
    pkill -f "_Server" 2>/dev/null
    pkill -f "_Client" 2>/dev/null
    sleep 1
}

# --- MAIN LOOP ---

cleanup

for IMPL in A1 A2 A3; do
  SERVER=$(find_bin "${IMPL}_Server")
  CLIENT=$(find_bin "${IMPL}_Client")

  if [[ -z "$SERVER" || -z "$CLIENT" ]]; then
      echo "[SKIP] Binary for $IMPL not found."
      continue
  fi

  chmod +x "$SERVER" "$CLIENT"

  for MSG in "${MSG_SIZES[@]}"; do
    for TH in "${THREADS[@]}"; do
      
      echo "----------------------------------------------------"
      echo "[RUN] $IMPL | Size: $MSG | Threads: $TH"

      # 1. Port Check
      while ss -lnt | grep -q ":$PORT "; do
          echo "   [WAIT] Port $PORT is busy..."
          sleep 1
      done

      # 2. Start Server
      ./"$SERVER" $PORT $MSG $TH > /dev/null 2>&1 &
      SPID=$!
      
      # 3. Wait for Server Ready
      SERVER_READY=0
      for i in {1..20}; do
          if ss -lnt | grep -q ":$PORT "; then
              SERVER_READY=1
              break
          fi
          sleep 0.1
      done

      if [ $SERVER_READY -eq 0 ]; then
          echo "   [FAIL] Server did not bind port $PORT."
          kill $SPID 2>/dev/null
          continue
      fi

      PERF_OUT=$(mktemp)
      CLIENT_OUT=$(mktemp)

      # 4. Run Client (Modified for Page Faults)
      # Replaced L1-dcache-load-misses with page-faults
      perf stat -x, \
        -e cycles,page-faults,cache-misses,context-switches \
        -- ./"$CLIENT" 127.0.0.1 $PORT $MSG $DURATION \
        > "$CLIENT_OUT" 2> "$PERF_OUT"

      kill $SPID 2>/dev/null
      wait $SPID 2>/dev/null

      # --- PARSING ---

      # Throughput
      BYTES=$(grep -i "BYTES" "$CLIENT_OUT" | tr -cd '0-9')
      BYTES=${BYTES:-0}
      THROUGHPUT=$(awk "BEGIN { printf \"%.6f\", ($BYTES*8)/($DURATION*1e9) }")

      # Latency
      RAW_LAT_LINE=$(grep -i -E "lat|avg|time" "$CLIENT_OUT" | head -n 1)
      LATENCY=$(echo "$RAW_LAT_LINE" | sed 's/[^0-9.]/ /g' | awk '{print $1}')
      LATENCY=${LATENCY:-0}

      # Perf Counters
      CYCLES=$(get_perf "cycles" "$PERF_OUT")
      
      # Now parsing page-faults instead of L1
      PAGE_FAULTS=$(get_perf "page-faults" "$PERF_OUT") 
      
      LLCMISS=$(get_perf "cache-misses" "$PERF_OUT")
      CTX=$(get_perf "context-switches" "$PERF_OUT")

      echo "$IMPL,$MSG,$TH,$THROUGHPUT,$LATENCY,$CYCLES,$PAGE_FAULTS,$LLCMISS,$CTX" >> "$RESULTS"
      echo "   -> Tput: $THROUGHPUT | Lat: $LATENCY | P.Faults: $PAGE_FAULTS"

      rm -f "$PERF_OUT" "$CLIENT_OUT"
    done
  done
done

echo "[DONE] Results written to $RESULTS"

