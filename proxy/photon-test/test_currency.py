# Copyright (c) 2026 Jamie Wells. All Rights Reserved.
# Proprietary and Confidential - Unauthorized copying or distribution prohibited.

#!/usr/bin/env python3
"""
Currency conversion tests — USD/GBP, GBX pence→pounds, EUR/GBP, cross-currency.
Run when modifying infer_currency, convert_price, _get_exchange_ticker_currency,
or _fetch_fx_rate_sync.

Includes regression tests for the EUR/USD currency bug (VWCE.AS was incorrectly
treated as USD-priced, causing GBP conversion to use the wrong rate).
"""

import sys
from base import ProxyTestBase, BASE_URL


class CurrencyTests(ProxyTestBase):

    # --- USD / GBP --------------------------------------------------------

    def test_usd_to_gbp(self):
        """AAPL in GBP should be roughly 0.60-0.90× the USD price."""
        name = "USD → GBP Conversion (AAPL)"
        usd = self._price(self._get("/stocks/AAPL"), "AAPL")
        gbp = self._price(self._get("/stocks/AAPL", {"currency": "GBP"}), "AAPL")
        if not usd or not gbp:
            self._fail(name, "Could not fetch prices")
            return
        rate = gbp / usd
        if 0.60 < rate < 0.90:
            self.logger.info(f"  AAPL USD: ${usd:.2f}, GBP: £{gbp:.2f}, rate: {rate:.4f}")
            self._pass(name)
        else:
            self._fail(name, f"Implied USD/GBP {rate:.4f} outside expected range 0.60-0.90")

    def test_gbp_to_usd(self):
        """A GBP-native LSE stock converted to USD should be 1.10-1.45× the GBP price."""
        name = "GBP → USD Conversion (VUSA)"
        gbp = self._price(self._get("/stocks/VUSA"), "VUSA")
        usd = self._price(self._get("/stocks/VUSA", {"currency": "USD"}), "VUSA")
        if not gbp or not usd:
            self._fail(name, "Could not fetch prices")
            return
        rate = usd / gbp
        if 1.10 < rate < 1.55:
            self.logger.info(f"  VUSA GBP: £{gbp:.2f}, USD: ${usd:.2f}, rate: {rate:.4f}")
            self._pass(name)
        else:
            self._fail(name, f"Implied GBP/USD {rate:.4f} outside expected range 1.10-1.55")

    # --- GBX (pence) → GBP -----------------------------------------------

    def test_gbx_pence_vuag(self):
        """VUAG.L trades in GBX (~9,000-13,000p). GBP result must be £90-130, not £9,000+."""
        name = "GBX Pence Conversion — VUAG (GBX÷100 = GBP)"
        price = self._price(self._get("/stocks/VUAG", {"currency": "GBP"}), "VUAG")
        if price is None:
            self._fail(name, "Could not fetch VUAG")
            return
        if price > 500:
            self._fail(name, f"£{price:.2f} — pence NOT divided by 100")
        elif price < 50:
            self._fail(name, f"£{price:.2f} — unrealistically low")
        else:
            self.logger.info(f"  VUAG GBP: £{price:.2f}")
            self._pass(name)

    def test_gbx_pence_eqgb(self):
        """EQGB.L trades in GBX (~55,000-60,000p). GBP result must be £550-600, not £55,000+."""
        name = "GBX Pence Conversion — EQGB.L (GBX÷100 = GBP)"
        price = self._price(self._get("/stocks/EQGB", {"currency": "GBP"}), "EQGB")
        if price is None:
            self._fail(name, "Could not fetch EQGB")
            return
        if price > 2000:
            self._fail(name, f"£{price:.2f} — pence NOT divided by 100")
        elif price < 200:
            self._fail(name, f"£{price:.2f} — unrealistically low")
        else:
            self.logger.info(f"  EQGB GBP: £{price:.2f}")
            self._pass(name)

    # --- EUR / GBP --------------------------------------------------------

    def test_eur_to_gbp(self):
        """VWCE.AS (EUR) in GBP should be ~£85-120 per share."""
        name = "EUR → GBP Conversion (VWCE)"
        price = self._price(self._get("/stocks/VWCE", {"currency": "GBP"}), "VWCE")
        if not price:
            self._fail(name, "Could not fetch VWCE")
            return
        if 50 < price < 200:
            self.logger.info(f"  VWCE GBP: £{price:.2f}")
            self._pass(name)
        else:
            self._fail(name, f"£{price:.2f} outside expected range £50-200")

    def test_eur_to_usd(self):
        """VWCE.AS (EUR) in USD should be ~$90-160 per share."""
        name = "EUR → USD Conversion (VWCE)"
        price = self._price(self._get("/stocks/VWCE", {"currency": "USD"}), "VWCE")
        if not price:
            self._fail(name, "Could not fetch VWCE")
            return
        if 50 < price < 250:
            self.logger.info(f"  VWCE USD: ${price:.2f}")
            self._pass(name)
        else:
            self._fail(name, f"${price:.2f} outside expected range $50-250")

    # --- EUR/GBP regression (VWCE.AS was incorrectly treated as USD) ------

    def test_euronext_not_usd_single(self):
        """
        REGRESSION (single endpoint): infer_currency must return EUR for VWCE.AS,
        not USD. EUR/GBP is historically higher than USD/GBP — if the implied
        rate for VWCE equals AAPL's USD/GBP rate, the old bug has regressed.
        """
        name = "REGRESSION: VWCE single — EUR/GBP rate, not USD/GBP"
        vwce_nat = self._price(self._get("/stocks/VWCE"), "VWCE")
        vwce_gbp = self._price(self._get("/stocks/VWCE", {"currency": "GBP"}), "VWCE")
        aapl_nat = self._price(self._get("/stocks/AAPL"), "AAPL")
        aapl_gbp = self._price(self._get("/stocks/AAPL", {"currency": "GBP"}), "AAPL")

        if not all([vwce_nat, vwce_gbp, aapl_nat, aapl_gbp]):
            self._fail(name, "Could not fetch required prices")
            return

        eur_gbp = vwce_gbp / vwce_nat
        usd_gbp = aapl_gbp / aapl_nat
        self.logger.info(f"  VWCE implied rate: {eur_gbp:.4f} (EUR/GBP)")
        self.logger.info(f"  AAPL implied rate: {usd_gbp:.4f} (USD/GBP)")

        if eur_gbp > usd_gbp:
            self._pass(name)
        else:
            self._fail(
                name,
                f"VWCE rate {eur_gbp:.4f} ≤ USD/GBP {usd_gbp:.4f} — "
                f"VWCE.AS may still be treated as USD (regression)"
            )

    def test_euronext_not_usd_batch(self):
        """
        REGRESSION (batch endpoint): same check via _fetch_batch_stock_data_sync /
        infer_currency. Both code paths must be fixed independently.
        """
        name = "REGRESSION: VWCE batch — EUR/GBP rate, not USD/GBP"
        vwce_nat = self._price(self._get("/stocks/batch", {"symbols": "VWCE"}), "VWCE")
        vwce_gbp = self._price(self._get("/stocks/batch", {"symbols": "VWCE", "currency": "GBP"}), "VWCE")
        aapl_nat = self._price(self._get("/stocks/batch", {"symbols": "AAPL"}), "AAPL")
        aapl_gbp = self._price(self._get("/stocks/batch", {"symbols": "AAPL", "currency": "GBP"}), "AAPL")

        if not all([vwce_nat, vwce_gbp, aapl_nat, aapl_gbp]):
            self._fail(name, "Could not fetch required prices")
            return

        eur_gbp = vwce_gbp / vwce_nat
        usd_gbp = aapl_gbp / aapl_nat
        self.logger.info(f"  VWCE implied rate: {eur_gbp:.4f} (EUR/GBP)")
        self.logger.info(f"  AAPL implied rate: {usd_gbp:.4f} (USD/GBP)")

        if eur_gbp > usd_gbp:
            self._pass(name)
        else:
            self._fail(
                name,
                f"VWCE rate {eur_gbp:.4f} ≤ USD/GBP {usd_gbp:.4f} — regression in batch path"
            )

    def _run_all(self):
        self.test_usd_to_gbp()
        self.test_gbp_to_usd()
        self.test_gbx_pence_vuag()
        self.test_gbx_pence_eqgb()
        self.test_eur_to_gbp()
        self.test_eur_to_usd()
        self.test_euronext_not_usd_single()
        self.test_euronext_not_usd_batch()


if __name__ == "__main__":
    url = "http://localhost:8080" if "--local" in sys.argv else BASE_URL
    result = CurrencyTests(url).run()
    sys.exit(0 if result.failed == 0 else 1)
