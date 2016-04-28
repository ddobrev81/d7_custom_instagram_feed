<?php

/**
 * @file
 * Contains PrpInstagramFeedFetcherbyHashtag and related classes.
 */

/**
 * Result of FeedsHTTPFetcher::fetch().
 */
class PrpInstagramFeedHashtagFetcherResult extends PrpInstagramFeedFetcherResult {

  private $hashtag;

  public function __construct($client_id, $hashtag) {
    $this->hashtag = $hashtag;
    $this->client_id = $client_id;

    try {
      $access_token = $this->getAccessToken($client_id);
      $this->access_token = $access_token;
    }
    catch (Exception $e) {
      drupal_set_message($e->getMessage(), 'error');
      prp_instagram_feed_watchdog_custom('Instagram Feeds', $e->getMessage(), NULL, WATCHDOG_ERROR);
    }

    $this->url = 'https://api.instagram.com/v1/tags/' . urlencode($this->hashtag) . '/media/recent?access_token=' . $access_token;

    // Get the min_id to be added to the request
    $min_ids = variable_get('prp_instagram_next_min_id', NULL);
    if ($min_ids != NULL) {
      $min_id = $min_ids[$this->hashtag];
      if ($min_id != NULL && !empty($min_id)) {
        $this->url .= '&min_tag_id=' . $min_id;
      }
    }

    FeedsFetcherResult::__construct('');
  }

  public function getHashtag() {
    return $this->hashtag;
  }
}

class PrpInstagramFeedFetcherbyHashtag extends PrpInstagramFeedFetcher {

  /**
   * Override  PrpInstagramFeedFetcher::fetch().
   */
  public function fetch(FeedsSource $source) {
    $source_config = $source->getConfigFor($this);

    return new PrpInstagramFeedHashtagFetcherResult($source_config['prp_instagram_feed_oauth_client_id'], $source_config['prp_instagram_feed_hashtag']);
  }

  /**
   * Override PrpInstagramFeedFetcher::sourceForm().
   */
  public function sourceForm($source_config) {
    $form = array();
    $form = parent::sourceForm($source_config);

    $form['prp_instagram_feed_hashtag'] = array(
      '#type' => 'textfield',
      '#title' => t('Hashtag'),
      '#default_value' => isset($source_config['prp_instagram_feed_hashtag']) ? $source_config['prp_instagram_feed_hashtag'] : '',
      '#description' => t('Hashtag to search for, without (#).'),
      '#required' => TRUE,
    );

    return $form;
  }

}
