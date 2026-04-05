
  var Module = typeof Module != 'undefined' ? Module : {};

  if (!Module['expectedDataFileDownloads']) Module['expectedDataFileDownloads'] = 0;
  Module['expectedDataFileDownloads']++;
  (() => {
    // Do not attempt to redownload the virtual filesystem data when in a pthread or a Wasm Worker context.
    var isPthread = typeof ENVIRONMENT_IS_PTHREAD != 'undefined' && ENVIRONMENT_IS_PTHREAD;
    var isWasmWorker = typeof ENVIRONMENT_IS_WASM_WORKER != 'undefined' && ENVIRONMENT_IS_WASM_WORKER;
    if (isPthread || isWasmWorker) return;
    var isNode = globalThis.process && globalThis.process.versions && globalThis.process.versions.node && globalThis.process.type != 'renderer';
    async function loadPackage(metadata) {

      var PACKAGE_PATH = '';
      if (typeof window === 'object') {
        PACKAGE_PATH = window['encodeURIComponent'](window.location.pathname.substring(0, window.location.pathname.lastIndexOf('/')) + '/');
      } else if (typeof process === 'undefined' && typeof location !== 'undefined') {
        // web worker
        PACKAGE_PATH = encodeURIComponent(location.pathname.substring(0, location.pathname.lastIndexOf('/')) + '/');
      }
      var PACKAGE_NAME = 'pkg_level_9.data';
      var REMOTE_PACKAGE_BASE = 'pkg_level_9.data';
      var REMOTE_PACKAGE_NAME = Module['locateFile'] ? Module['locateFile'](REMOTE_PACKAGE_BASE, '') : REMOTE_PACKAGE_BASE;
      var REMOTE_PACKAGE_SIZE = metadata['remote_package_size'];

      async function fetchRemotePackage(packageName, packageSize) {
        if (isNode) {
          var contents = require('fs').readFileSync(packageName);
          return new Uint8Array(contents).buffer;
        }
        if (!Module['dataFileDownloads']) Module['dataFileDownloads'] = {};
        try {
          var response = await fetch(packageName);
        } catch (e) {
          throw new Error(`Network Error: ${packageName}`, {e});
        }
        if (!response.ok) {
          throw new Error(`${response.status}: ${response.url}`);
        }

        const chunks = [];
        const headers = response.headers;
        const total = Number(headers.get('Content-Length') || packageSize);
        let loaded = 0;

        Module['setStatus'] && Module['setStatus']('Downloading data...');
        const reader = response.body.getReader();

        while (1) {
          var {done, value} = await reader.read();
          if (done) break;
          chunks.push(value);
          loaded += value.length;
          Module['dataFileDownloads'][packageName] = {loaded, total};

          let totalLoaded = 0;
          let totalSize = 0;

          for (const download of Object.values(Module['dataFileDownloads'])) {
            totalLoaded += download.loaded;
            totalSize += download.total;
          }

          Module['setStatus'] && Module['setStatus'](`Downloading data... (${totalLoaded}/${totalSize})`);
        }

        const packageData = new Uint8Array(chunks.map((c) => c.length).reduce((a, b) => a + b, 0));
        let offset = 0;
        for (const chunk of chunks) {
          packageData.set(chunk, offset);
          offset += chunk.length;
        }
        return packageData.buffer;
      }

      var fetchPromise;
      var fetched = Module['getPreloadedPackage'] && Module['getPreloadedPackage'](REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE);

      if (!fetched) {
        // Note that we don't use await here because we want to execute the
        // the rest of this function immediately.
        fetchPromise = fetchRemotePackage(REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE);
      }

    async function runWithFS(Module) {

      function assert(check, msg) {
        if (!check) throw new Error(msg);
      }
Module['FS_createPath']("/", "CandyCrisisResources", true, true);
Module['FS_createPath']("/CandyCrisisResources", "PICT_5009", true, true);

    for (var file of metadata['files']) {
      var name = file['filename']
      Module['addRunDependency'](`fp ${name}`);
    }

      async function processPackageData(arrayBuffer) {
        assert(arrayBuffer, 'Loading data file failed.');
        assert(arrayBuffer.constructor.name === ArrayBuffer.name, 'bad input to processPackageData ' + arrayBuffer.constructor.name);
        var byteArray = new Uint8Array(arrayBuffer);
        var curr;
        // Reuse the bytearray from the XHR as the source for file reads.
          for (var file of metadata['files']) {
            var name = file['filename'];
            var data = byteArray.subarray(file['start'], file['end']);
            // canOwn this data in the filesystem, it is a slice into the heap that will never change
        Module['FS_createDataFile'](name, null, data, true, true, true);
        Module['removeRunDependency'](`fp ${name}`);
          }
          Module['removeRunDependency']('datafile_pkg_level_9.data');
      }
      Module['addRunDependency']('datafile_pkg_level_9.data');

      if (!Module['preloadResults']) Module['preloadResults'] = {};

      Module['preloadResults'][PACKAGE_NAME] = {fromCache: false};
      if (!fetched) {
        fetched = await fetchPromise;
      }
      processPackageData(fetched);

    }
    if (Module['calledRun']) {
      runWithFS(Module);
    } else {
      if (!Module['preRun']) Module['preRun'] = [];
      Module['preRun'].push(runWithFS); // FS is not initialized yet, wait for it
    }

    }
    loadPackage({"files": [{"filename": "/CandyCrisisResources/PICT_5009/Backdrop.png", "start": 0, "end": 191583}, {"filename": "/CandyCrisisResources/PICT_5009/Board.png", "start": 191583, "end": 197183}, {"filename": "/CandyCrisisResources/PICT_5009/BoardGrays.png", "start": 197183, "end": 199038}, {"filename": "/CandyCrisisResources/PICT_5009/BoardNext.png", "start": 199038, "end": 203012}, {"filename": "/CandyCrisisResources/PICT_5009/BoardScore.png", "start": 203012, "end": 204729}, {"filename": "/CandyCrisisResources/PICT_5009/BoardSelect.png", "start": 204729, "end": 265106}, {"filename": "/CandyCrisisResources/PICT_5009/H-1A.png", "start": 265106, "end": 274224}, {"filename": "/CandyCrisisResources/PICT_5009/H-2A.png", "start": 274224, "end": 286274}, {"filename": "/CandyCrisisResources/PICT_5009/H-3A.png", "start": 286274, "end": 300954}, {"filename": "/CandyCrisisResources/PICT_5009/H-4A.png", "start": 300954, "end": 309311}, {"filename": "/CandyCrisisResources/PICT_5009/H-5A.png", "start": 309311, "end": 311704}, {"filename": "/CandyCrisisResources/PICT_5009/H-6A.png", "start": 311704, "end": 326384}, {"filename": "/CandyCrisisResources/PICT_5009/N-1A.png", "start": 326384, "end": 335768}, {"filename": "/CandyCrisisResources/PICT_5009/N-2A.png", "start": 335768, "end": 345295}, {"filename": "/CandyCrisisResources/PICT_5009/N-3A.png", "start": 345295, "end": 354887}, {"filename": "/CandyCrisisResources/PICT_5009/N-4A.png", "start": 354887, "end": 363680}, {"filename": "/CandyCrisisResources/PICT_5009/N-5A.png", "start": 363680, "end": 365885}, {"filename": "/CandyCrisisResources/PICT_5009/S-1A.png", "start": 365885, "end": 371137}, {"filename": "/CandyCrisisResources/PICT_5009/S-2A.png", "start": 371137, "end": 374252}, {"filename": "/CandyCrisisResources/PICT_5009/S-3A.png", "start": 374252, "end": 377337}, {"filename": "/CandyCrisisResources/PICT_5009/S-4A.png", "start": 377337, "end": 383916}, {"filename": "/CandyCrisisResources/PICT_5009/S-5A.png", "start": 383916, "end": 392804}, {"filename": "/CandyCrisisResources/PICT_5009/S-6A.png", "start": 392804, "end": 396519}, {"filename": "/CandyCrisisResources/PICT_5009/S-6B.png", "start": 396519, "end": 403098}, {"filename": "/CandyCrisisResources/PICT_5009/S-6C.png", "start": 403098, "end": 411793}, {"filename": "/CandyCrisisResources/PICT_5009/S-6D.png", "start": 411793, "end": 418372}, {"filename": "/CandyCrisisResources/PICT_5009/Score.png", "start": 418372, "end": 426250}, {"filename": "/CandyCrisisResources/PICT_5009/ScoreInnerMask.png", "start": 426250, "end": 427348}, {"filename": "/CandyCrisisResources/PICT_5009/ScoreOuterMask.png", "start": 427348, "end": 428477}, {"filename": "/CandyCrisisResources/PICT_5009/X-UA.png", "start": 428477, "end": 563831}, {"filename": "/CandyCrisisResources/PICT_5009/X-UB.png", "start": 563831, "end": 701742}, {"filename": "/CandyCrisisResources/PICT_5009/X-UC.png", "start": 701742, "end": 840280}, {"filename": "/CandyCrisisResources/PICT_5009/X-UD.png", "start": 840280, "end": 983428}, {"filename": "/CandyCrisisResources/PICT_5009/X-UE.png", "start": 983428, "end": 1124403}, {"filename": "/CandyCrisisResources/PICT_5009/X-UF.png", "start": 1124403, "end": 1267020}, {"filename": "/CandyCrisisResources/PICT_5009/X-UG.png", "start": 1267020, "end": 1408358}, {"filename": "/CandyCrisisResources/PICT_5009/X-UH.png", "start": 1408358, "end": 1548023}, {"filename": "/CandyCrisisResources/mod_137", "start": 1548023, "end": 2099353}], "remote_package_size": 2099353});

  })();
