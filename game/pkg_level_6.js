
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
      var PACKAGE_NAME = 'pkg_level_6.data';
      var REMOTE_PACKAGE_BASE = 'pkg_level_6.data';
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
Module['FS_createPath']("/CandyCrisisResources", "PICT_5006", true, true);

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
          Module['removeRunDependency']('datafile_pkg_level_6.data');
      }
      Module['addRunDependency']('datafile_pkg_level_6.data');

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
    loadPackage({"files": [{"filename": "/CandyCrisisResources/PICT_5006/Backdrop.png", "start": 0, "end": 170396}, {"filename": "/CandyCrisisResources/PICT_5006/Board.png", "start": 170396, "end": 175996}, {"filename": "/CandyCrisisResources/PICT_5006/BoardGrays.png", "start": 175996, "end": 177851}, {"filename": "/CandyCrisisResources/PICT_5006/BoardNext.png", "start": 177851, "end": 181817}, {"filename": "/CandyCrisisResources/PICT_5006/BoardScore.png", "start": 181817, "end": 183534}, {"filename": "/CandyCrisisResources/PICT_5006/BoardSelect.png", "start": 183534, "end": 243945}, {"filename": "/CandyCrisisResources/PICT_5006/H-1A.png", "start": 243945, "end": 323552}, {"filename": "/CandyCrisisResources/PICT_5006/H-2A.png", "start": 323552, "end": 403188}, {"filename": "/CandyCrisisResources/PICT_5006/H-3A.png", "start": 403188, "end": 482824}, {"filename": "/CandyCrisisResources/PICT_5006/H-4A.png", "start": 482824, "end": 563764}, {"filename": "/CandyCrisisResources/PICT_5006/H-5A.png", "start": 563764, "end": 643668}, {"filename": "/CandyCrisisResources/PICT_5006/H-6A.png", "start": 643668, "end": 723275}, {"filename": "/CandyCrisisResources/PICT_5006/N-1A.png", "start": 723275, "end": 807169}, {"filename": "/CandyCrisisResources/PICT_5006/N-2A.png", "start": 807169, "end": 891052}, {"filename": "/CandyCrisisResources/PICT_5006/N-3A.png", "start": 891052, "end": 974973}, {"filename": "/CandyCrisisResources/PICT_5006/N-4A.png", "start": 974973, "end": 1059999}, {"filename": "/CandyCrisisResources/PICT_5006/N-5A.png", "start": 1059999, "end": 1144064}, {"filename": "/CandyCrisisResources/PICT_5006/S-1A.png", "start": 1144064, "end": 1226024}, {"filename": "/CandyCrisisResources/PICT_5006/S-2A.png", "start": 1226024, "end": 1307983}, {"filename": "/CandyCrisisResources/PICT_5006/S-3A.png", "start": 1307983, "end": 1389963}, {"filename": "/CandyCrisisResources/PICT_5006/S-4A.png", "start": 1389963, "end": 1472709}, {"filename": "/CandyCrisisResources/PICT_5006/S-5A.png", "start": 1472709, "end": 1554438}, {"filename": "/CandyCrisisResources/PICT_5006/S-6A.png", "start": 1554438, "end": 1636397}, {"filename": "/CandyCrisisResources/PICT_5006/Score.png", "start": 1636397, "end": 1644295}, {"filename": "/CandyCrisisResources/PICT_5006/ScoreInnerMask.png", "start": 1644295, "end": 1645393}, {"filename": "/CandyCrisisResources/PICT_5006/ScoreOuterMask.png", "start": 1645393, "end": 1646522}, {"filename": "/CandyCrisisResources/mod_134", "start": 1646522, "end": 1800170}], "remote_package_size": 1800170});

  })();
