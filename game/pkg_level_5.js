
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
      var PACKAGE_NAME = 'pkg_level_5.data';
      var REMOTE_PACKAGE_BASE = 'pkg_level_5.data';
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
Module['FS_createPath']("/CandyCrisisResources", "PICT_5005", true, true);

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
          Module['removeRunDependency']('datafile_pkg_level_5.data');
      }
      Module['addRunDependency']('datafile_pkg_level_5.data');

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
    loadPackage({"files": [{"filename": "/CandyCrisisResources/PICT_5005/Backdrop.png", "start": 0, "end": 238626}, {"filename": "/CandyCrisisResources/PICT_5005/Board.png", "start": 238626, "end": 244226}, {"filename": "/CandyCrisisResources/PICT_5005/BoardGrays.png", "start": 244226, "end": 246081}, {"filename": "/CandyCrisisResources/PICT_5005/BoardNext.png", "start": 246081, "end": 250050}, {"filename": "/CandyCrisisResources/PICT_5005/BoardScore.png", "start": 250050, "end": 251767}, {"filename": "/CandyCrisisResources/PICT_5005/BoardSelect.png", "start": 251767, "end": 312239}, {"filename": "/CandyCrisisResources/PICT_5005/H-1A.png", "start": 312239, "end": 399900}, {"filename": "/CandyCrisisResources/PICT_5005/H-2A.png", "start": 399900, "end": 487309}, {"filename": "/CandyCrisisResources/PICT_5005/H-3A.png", "start": 487309, "end": 574828}, {"filename": "/CandyCrisisResources/PICT_5005/H-4A.png", "start": 574828, "end": 661988}, {"filename": "/CandyCrisisResources/PICT_5005/H-5A.png", "start": 661988, "end": 748033}, {"filename": "/CandyCrisisResources/PICT_5005/H-6A.png", "start": 748033, "end": 835694}, {"filename": "/CandyCrisisResources/PICT_5005/N-1A.png", "start": 835694, "end": 924026}, {"filename": "/CandyCrisisResources/PICT_5005/N-2A.png", "start": 924026, "end": 1012474}, {"filename": "/CandyCrisisResources/PICT_5005/N-3A.png", "start": 1012474, "end": 1101039}, {"filename": "/CandyCrisisResources/PICT_5005/N-4A.png", "start": 1101039, "end": 1189216}, {"filename": "/CandyCrisisResources/PICT_5005/N-5A.png", "start": 1189216, "end": 1275381}, {"filename": "/CandyCrisisResources/PICT_5005/S-1A.png", "start": 1275381, "end": 1363103}, {"filename": "/CandyCrisisResources/PICT_5005/S-2A.png", "start": 1363103, "end": 1451228}, {"filename": "/CandyCrisisResources/PICT_5005/S-3A.png", "start": 1451228, "end": 1539151}, {"filename": "/CandyCrisisResources/PICT_5005/S-4A.png", "start": 1539151, "end": 1626220}, {"filename": "/CandyCrisisResources/PICT_5005/S-5A.png", "start": 1626220, "end": 1711654}, {"filename": "/CandyCrisisResources/PICT_5005/S-6A.png", "start": 1711654, "end": 1799577}, {"filename": "/CandyCrisisResources/PICT_5005/Score.png", "start": 1799577, "end": 1807462}, {"filename": "/CandyCrisisResources/PICT_5005/ScoreInnerMask.png", "start": 1807462, "end": 1808560}, {"filename": "/CandyCrisisResources/PICT_5005/ScoreOuterMask.png", "start": 1808560, "end": 1809689}, {"filename": "/CandyCrisisResources/mod_133", "start": 1809689, "end": 2093992}], "remote_package_size": 2093992});

  })();
