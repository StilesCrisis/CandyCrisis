
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
      var PACKAGE_NAME = '/Users/johnstiles/Candy Crisis/build-web/pkg_level_8.data';
      var REMOTE_PACKAGE_BASE = 'pkg_level_8.data';
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
Module['FS_createPath']("/CandyCrisisResources", "PICT_5008", true, true);

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
          Module['removeRunDependency']('datafile_/Users/johnstiles/Candy Crisis/build-web/pkg_level_8.data');
      }
      Module['addRunDependency']('datafile_/Users/johnstiles/Candy Crisis/build-web/pkg_level_8.data');

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
    loadPackage({"files": [{"filename": "/CandyCrisisResources/PICT_5008/Backdrop.png", "start": 0, "end": 386419}, {"filename": "/CandyCrisisResources/PICT_5008/Board.png", "start": 386419, "end": 392019}, {"filename": "/CandyCrisisResources/PICT_5008/BoardGrays.png", "start": 392019, "end": 393874}, {"filename": "/CandyCrisisResources/PICT_5008/BoardNext.png", "start": 393874, "end": 397848}, {"filename": "/CandyCrisisResources/PICT_5008/BoardScore.png", "start": 397848, "end": 399565}, {"filename": "/CandyCrisisResources/PICT_5008/BoardSelect.png", "start": 399565, "end": 459942}, {"filename": "/CandyCrisisResources/PICT_5008/H-1A.png", "start": 459942, "end": 463557}, {"filename": "/CandyCrisisResources/PICT_5008/H-2A.png", "start": 463557, "end": 466513}, {"filename": "/CandyCrisisResources/PICT_5008/H-3A.png", "start": 466513, "end": 470443}, {"filename": "/CandyCrisisResources/PICT_5008/H-4A.png", "start": 470443, "end": 474427}, {"filename": "/CandyCrisisResources/PICT_5008/H-5A.png", "start": 474427, "end": 478406}, {"filename": "/CandyCrisisResources/PICT_5008/H-6A.png", "start": 478406, "end": 482385}, {"filename": "/CandyCrisisResources/PICT_5008/N-1A.png", "start": 482385, "end": 483895}, {"filename": "/CandyCrisisResources/PICT_5008/N-2A.png", "start": 483895, "end": 485386}, {"filename": "/CandyCrisisResources/PICT_5008/N-3A.png", "start": 485386, "end": 486865}, {"filename": "/CandyCrisisResources/PICT_5008/N-4A.png", "start": 486865, "end": 488118}, {"filename": "/CandyCrisisResources/PICT_5008/N-5A.png", "start": 488118, "end": 489008}, {"filename": "/CandyCrisisResources/PICT_5008/S-1A.png", "start": 489008, "end": 491349}, {"filename": "/CandyCrisisResources/PICT_5008/S-2A.png", "start": 491349, "end": 493660}, {"filename": "/CandyCrisisResources/PICT_5008/S-3A.png", "start": 493660, "end": 496003}, {"filename": "/CandyCrisisResources/PICT_5008/S-4A.png", "start": 496003, "end": 499130}, {"filename": "/CandyCrisisResources/PICT_5008/S-5A.png", "start": 499130, "end": 501751}, {"filename": "/CandyCrisisResources/PICT_5008/S-6A.png", "start": 501751, "end": 504092}, {"filename": "/CandyCrisisResources/PICT_5008/Score.png", "start": 504092, "end": 511996}, {"filename": "/CandyCrisisResources/PICT_5008/ScoreInnerMask.png", "start": 511996, "end": 513094}, {"filename": "/CandyCrisisResources/PICT_5008/ScoreOuterMask.png", "start": 513094, "end": 514223}, {"filename": "/CandyCrisisResources/PICT_5008/X-UA.png", "start": 514223, "end": 542107}, {"filename": "/CandyCrisisResources/PICT_5008/X-UB.png", "start": 542107, "end": 568237}, {"filename": "/CandyCrisisResources/PICT_5008/X-UC.png", "start": 568237, "end": 592820}, {"filename": "/CandyCrisisResources/PICT_5008/X-UD.png", "start": 592820, "end": 615188}, {"filename": "/CandyCrisisResources/PICT_5008/X-UE.png", "start": 615188, "end": 636244}, {"filename": "/CandyCrisisResources/PICT_5008/X-UF.png", "start": 636244, "end": 656514}, {"filename": "/CandyCrisisResources/PICT_5008/X-UG.png", "start": 656514, "end": 680463}, {"filename": "/CandyCrisisResources/mod_136", "start": 680463, "end": 880425}], "remote_package_size": 880425});

  })();
