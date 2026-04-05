
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
      var PACKAGE_NAME = '/Users/johnstiles/Candy Crisis/build-web/pkg_level_1.data';
      var REMOTE_PACKAGE_BASE = 'pkg_level_1.data';
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
Module['FS_createPath']("/CandyCrisisResources", "PICT_5001", true, true);

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
          Module['removeRunDependency']('datafile_/Users/johnstiles/Candy Crisis/build-web/pkg_level_1.data');
      }
      Module['addRunDependency']('datafile_/Users/johnstiles/Candy Crisis/build-web/pkg_level_1.data');

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
    loadPackage({"files": [{"filename": "/CandyCrisisResources/PICT_5001/Backdrop.png", "start": 0, "end": 244731}, {"filename": "/CandyCrisisResources/PICT_5001/Board.png", "start": 244731, "end": 250331}, {"filename": "/CandyCrisisResources/PICT_5001/BoardGrays.png", "start": 250331, "end": 252186}, {"filename": "/CandyCrisisResources/PICT_5001/BoardNext.png", "start": 252186, "end": 256149}, {"filename": "/CandyCrisisResources/PICT_5001/BoardScore.png", "start": 256149, "end": 257866}, {"filename": "/CandyCrisisResources/PICT_5001/BoardSelect.png", "start": 257866, "end": 318104}, {"filename": "/CandyCrisisResources/PICT_5001/H-1A.png", "start": 318104, "end": 395627}, {"filename": "/CandyCrisisResources/PICT_5001/H-2A.png", "start": 395627, "end": 473076}, {"filename": "/CandyCrisisResources/PICT_5001/H-3A.png", "start": 473076, "end": 550738}, {"filename": "/CandyCrisisResources/PICT_5001/H-4A.png", "start": 550738, "end": 629628}, {"filename": "/CandyCrisisResources/PICT_5001/H-5A.png", "start": 629628, "end": 709256}, {"filename": "/CandyCrisisResources/PICT_5001/H-6A.png", "start": 709256, "end": 788146}, {"filename": "/CandyCrisisResources/PICT_5001/N-1A.png", "start": 788146, "end": 865743}, {"filename": "/CandyCrisisResources/PICT_5001/N-2A.png", "start": 865743, "end": 943266}, {"filename": "/CandyCrisisResources/PICT_5001/N-3A.png", "start": 943266, "end": 1020340}, {"filename": "/CandyCrisisResources/PICT_5001/N-4A.png", "start": 1020340, "end": 1097876}, {"filename": "/CandyCrisisResources/PICT_5001/N-5A.png", "start": 1097876, "end": 1172762}, {"filename": "/CandyCrisisResources/PICT_5001/S-1A.png", "start": 1172762, "end": 1249973}, {"filename": "/CandyCrisisResources/PICT_5001/S-2A.png", "start": 1249973, "end": 1327813}, {"filename": "/CandyCrisisResources/PICT_5001/S-3A.png", "start": 1327813, "end": 1405561}, {"filename": "/CandyCrisisResources/PICT_5001/S-4A.png", "start": 1405561, "end": 1482955}, {"filename": "/CandyCrisisResources/PICT_5001/S-5A.png", "start": 1482955, "end": 1560387}, {"filename": "/CandyCrisisResources/PICT_5001/S-6A.png", "start": 1560387, "end": 1637819}, {"filename": "/CandyCrisisResources/PICT_5001/Score.png", "start": 1637819, "end": 1645546}, {"filename": "/CandyCrisisResources/PICT_5001/ScoreInnerMask.png", "start": 1645546, "end": 1646644}, {"filename": "/CandyCrisisResources/PICT_5001/ScoreOuterMask.png", "start": 1646644, "end": 1647773}, {"filename": "/CandyCrisisResources/mod_129", "start": 1647773, "end": 2036239}], "remote_package_size": 2036239});

  })();
