
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
      var PACKAGE_NAME = '/Users/johnstiles/Candy Crisis/build-web/pkg_level_10.data';
      var REMOTE_PACKAGE_BASE = 'pkg_level_10.data';
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
Module['FS_createPath']("/CandyCrisisResources", "PICT_5010", true, true);

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
          Module['removeRunDependency']('datafile_/Users/johnstiles/Candy Crisis/build-web/pkg_level_10.data');
      }
      Module['addRunDependency']('datafile_/Users/johnstiles/Candy Crisis/build-web/pkg_level_10.data');

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
    loadPackage({"files": [{"filename": "/CandyCrisisResources/PICT_5010/Backdrop.png", "start": 0, "end": 193637}, {"filename": "/CandyCrisisResources/PICT_5010/Board.png", "start": 193637, "end": 199237}, {"filename": "/CandyCrisisResources/PICT_5010/BoardGrays.png", "start": 199237, "end": 201092}, {"filename": "/CandyCrisisResources/PICT_5010/BoardNext.png", "start": 201092, "end": 205046}, {"filename": "/CandyCrisisResources/PICT_5010/BoardScore.png", "start": 205046, "end": 206763}, {"filename": "/CandyCrisisResources/PICT_5010/BoardSelect.png", "start": 206763, "end": 264751}, {"filename": "/CandyCrisisResources/PICT_5010/CandyRain1.png", "start": 264751, "end": 291283}, {"filename": "/CandyCrisisResources/PICT_5010/CandyRain2.png", "start": 291283, "end": 317695}, {"filename": "/CandyCrisisResources/PICT_5010/CandyRain3.png", "start": 317695, "end": 344299}, {"filename": "/CandyCrisisResources/PICT_5010/CandyRain4.png", "start": 344299, "end": 370829}, {"filename": "/CandyCrisisResources/PICT_5010/H-1A.png", "start": 370829, "end": 456194}, {"filename": "/CandyCrisisResources/PICT_5010/H-2A.png", "start": 456194, "end": 546105}, {"filename": "/CandyCrisisResources/PICT_5010/H-3A.png", "start": 546105, "end": 635347}, {"filename": "/CandyCrisisResources/PICT_5010/H-4A.png", "start": 635347, "end": 721311}, {"filename": "/CandyCrisisResources/PICT_5010/H-5A.png", "start": 721311, "end": 814838}, {"filename": "/CandyCrisisResources/PICT_5010/H-6A.png", "start": 814838, "end": 904080}, {"filename": "/CandyCrisisResources/PICT_5010/N-1A.png", "start": 904080, "end": 981136}, {"filename": "/CandyCrisisResources/PICT_5010/N-2A.png", "start": 981136, "end": 1058231}, {"filename": "/CandyCrisisResources/PICT_5010/N-3A.png", "start": 1058231, "end": 1134951}, {"filename": "/CandyCrisisResources/PICT_5010/N-4A.png", "start": 1134951, "end": 1213548}, {"filename": "/CandyCrisisResources/PICT_5010/N-5A.png", "start": 1213548, "end": 1296579}, {"filename": "/CandyCrisisResources/PICT_5010/S-1A.png", "start": 1296579, "end": 1381855}, {"filename": "/CandyCrisisResources/PICT_5010/S-2A.png", "start": 1381855, "end": 1466994}, {"filename": "/CandyCrisisResources/PICT_5010/S-3A.png", "start": 1466994, "end": 1552498}, {"filename": "/CandyCrisisResources/PICT_5010/S-4A.png", "start": 1552498, "end": 1639969}, {"filename": "/CandyCrisisResources/PICT_5010/S-5A.png", "start": 1639969, "end": 1730542}, {"filename": "/CandyCrisisResources/PICT_5010/S-6A.png", "start": 1730542, "end": 1815818}, {"filename": "/CandyCrisisResources/PICT_5010/Score.png", "start": 1815818, "end": 1823649}, {"filename": "/CandyCrisisResources/PICT_5010/ScoreInnerMask.png", "start": 1823649, "end": 1824747}, {"filename": "/CandyCrisisResources/PICT_5010/ScoreOuterMask.png", "start": 1824747, "end": 1825876}, {"filename": "/CandyCrisisResources/mod_138", "start": 1825876, "end": 2124746}], "remote_package_size": 2124746});

  })();
